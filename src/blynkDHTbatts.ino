// blynkDHTbatts
// ESP8266 + DHTXX + deepsleep + battery monitor + OTA + Blynk
// by: Truglodite

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <BlynkSimpleEsp8266_SSL.h>
#include <DHT.h>
extern "C" {
  #include "user_interface.h"
}
#define privacy
#include "configuration.h"
#ifdef privacy
  #include "privacy.h"
#endif

//#define Debug                                   // Uncomment to enable serial debug output

#ifdef batteryMonitor
  float vbatt = 0;                                // Holder for battery voltage
#endif
bool firmwareUp = 0;                              // Holder for firmware button position
bool isFirmwareUpSet = 0;                         // Holder for sync finished/unfinished status
bool OTAnotificationSent = 0;                     // Holder for OTA ready notification
bool batteryLow = 0;                              // Low battery flag
unsigned long dhtStartTime = 0;                   // Holder for DHT timeout timer
unsigned long otaStartTime = 0;                   // Holder for OTA timeout timer
float t = 0;                                      // Holder for temperature
float h = 0;                                      // Holder for humidity
float hif = 0;                                    // Holder for heat index
float dpf = 0;                                    // Holder for dew point
int retries = 0;                                  // Holder for DHT read retry count

IPAddress myIP;
ESP8266WebServer httpServer(80);  //init web update server
ESP8266HTTPUpdateServer httpUpdater;

DHT dht(dhtPin,dhtType);

char notifyOTAready[sizeof(notifyOTAreadyX) + 27 + sizeof(update_path) + 1] = {0};
///////////////////////////////////////////////////////////////
//************************  SETUP  **************************//
void setup() {
  pinMode(dhtPowerPin,OUTPUT);             // Setup sensor power switch
  digitalWrite(dhtPowerPin,LOW);           // Make sure DHT is off

  WiFi.mode(WIFI_OFF);                     // Make sure wifi is off so we get a clean adc read
  WiFi.forceSleepBegin();
  delay(1);

  #ifdef batteryMonitor                    // Check Battery
    delay(100);                            // Help with analog read stability?
    //vbatt = system_adc_read();
    vbatt = analogRead(A0);
    vbatt = vbatt * vbattRatio / 1023.0;   // Calc battery voltage on a 10bit scale    if(vbatt < vbattCrit) { // vbatt critical, shutdown immediately
    if(vbatt < vbattCrit) { // vbatt critical, shutdown immediately
      #ifdef debug
        Serial.println("Critical Battery Shut Down...");
      #endif
      ESP.deepSleep(0,WAKE_RF_DISABLED);// We won't turn back on until batt is replaced (reset)
      yield();
    }
    if(vbatt <= vbattLow) batteryLow = 1;
  #endif

  digitalWrite(dhtPowerPin,HIGH);          // Turn on sensor power
  dht.begin();
  dhtStartTime = millis();
  while(millis() - dhtStartTime < dhtTimeout);// Wait before reading
  h = dht.readHumidity();                  // RH %
  t = dht.readTemperature(true);           // Temp Farenheit True
  while(isnan(h) || isnan(t)) {            // Retry reading(s) if we get any NAN's
    delay(dhtTimeout);
    if(isnan(h)) {h = dht.readHumidity();};
    if(isnan(t)) {t = dht.readTemperature(true);};
    retries ++;
    if(retries > retriesMax)  {            // We've retried too many times, reboot
      digitalWrite(dhtPowerPin,LOW);       // This is a case that could 'quiet bootloop'!!!
      ESP.deepSleep(60000,WAKE_RF_DISABLED);// We risk this to avoid wifi mucking the DHT measurements :/
      yield();
    }
  }
  digitalWrite(dhtPowerPin,LOW);           // Done reading sensor, turn it off
  hif = dht.computeHeatIndex(t, h);        // Calc Heat Index - Farenheit
  dpf = dewPointFarenheit(t,h);            // Calc Dew Point - Farenheit

  WiFi.forceSleepWake();                   // Turn wifi on
  WiFi.mode(WIFI_STA);                     // STA mode saves batteries?
  //wifi_set_macaddr(STATION_IF, mac);     // Set wifi mac address
  //WiFi.config(ssid,pass,staticIP)        // Config wifi with static IP
  wifi_station_connect();                  // Connect to wifi?
  //WiFi.begin();

  Blynk.begin(auth,ssid,pass);             // Default Blynk server
  //Blynk.begin(auth,ssid,pass,staticIP);  // Use a private Blynk server instead
  //Blynk.config(auth);                    // Use with manual wifi management
  while (Blynk.connect() == false) {       // Loop until connected
    if(millis()>5000) {                    // Abort and reboot if not connected within 5sec
      digitalWrite(dhtPowerPin,LOW);
      ESP.deepSleep(60000,WAKE_RF_DISABLED);// Try again in 1min
      delay(500);
    }
  }

  #ifdef debug
    Serial.println("Blynk Connected");
  #endif
  myIP = WiFi.localIP();

  // combine this string now that we have an IP
  sprintf(notifyOTAready, "OTA Waiting...\nhttp://%s%s", myIP.toString().c_str(), update_path);

  //Initialize OTA server
  #ifdef debug
    Serial.print("Initializing OTA: ");
  #endif
  MDNS.begin(hostName);
  httpUpdater.setup(&httpServer, update_path, update_username, otaPassword);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  #ifdef debug
    Serial.println("Ready");
  #endif

}
//********************  END SETUP  **************************//
///////////////////////////////////////////////////////////////


BLYNK_CONNECTED() {         // Sync when Blynk is connected
  Blynk.syncAll();
}

BLYNK_WRITE(firmwareVpin) { // Sync firmware button from app, and set the flag
  firmwareUp = param.asInt();
  isFirmwareUpSet = 1;
}

///////////////////////////////////////////////////////////////
//************************  LOOP  ***************************//
void loop(){
  // Always run() to allow exiting OTA mode from the app after boot up.
  // Otherwise if you don't turn it "off" before OTA reboots, a hard reset (or another OTA upload) is required.
  Blynk.run();

  // FW button off: normal routine... upload data, deepsleep
  if(isFirmwareUpSet && !firmwareUp){
    uploadData();
  }

  // FW button on & battery ok: send one notification, set flag, start timer
  else if(isFirmwareUpSet && firmwareUp && !OTAnotificationSent && !batteryLow){
    Blynk.notify(notifyOTAready);
    OTAnotificationSent = 1;
    otaStartTime = millis();
  }
  // FW button on & battery low: send a different notification
  else if(isFirmwareUpSet && firmwareUp && !OTAnotificationSent && batteryLow){
    Blynk.notify(notifyLowBatt);
    OTAnotificationSent = 1;
  }

  // FW button on & battery OK: handle OTA calls
  else if(isFirmwareUpSet && firmwareUp && !batteryLow) {
    httpServer.handleClient();
    MDNS.update();
    if(millis() - otaStartTime > otaTimeout)  {// OTA timeout... reset so we send another notification
      digitalWrite(dhtPowerPin,LOW);
      ESP.deepSleep(1000,WAKE_RF_DISABLED);
      delay(500);
    }
  }
}
//**********************  END LOOP  *************************//
///////////////////////////////////////////////////////////////

// The main routine that runs after the Vpins are downloaded
void uploadData()  {
  #ifdef extraOutput
    float rssi =  WiFi.RSSI();              // RSSI in dBm
  #endif

  // Write data to virtual pins
  Blynk.virtualWrite(tempVpin, t);
  Blynk.virtualWrite(humidVpin, h);
  Blynk.virtualWrite(indexVpin, hif);
  Blynk.virtualWrite(dewpointVpin, dpf);
  #ifdef extraOutput
    Blynk.virtualWrite(rssiVpin, rssi);
  #endif

  #ifdef batteryMonitor
    Blynk.virtualWrite(battVpin, vbatt);
    if(vbatt <= vbattLow) {               // Battery low
      Blynk.notify(notifyLowBatt);
      #ifdef extraOutput
        Blynk.virtualWrite(millisVpin, millis());
      #endif
      ESP.deepSleep(longSleepMicros,WAKE_RF_DISABLED);
      yield();
    }
  #endif

  #ifdef extraOutput
    Blynk.virtualWrite(millisVpin, millis());// Main routine done, sleep for the usual time
  #endif

  ESP.deepSleep(sleepMicros,WAKE_RF_DISABLED);
  yield();
}

// Takes Fareneit, and returns dewpoint in Farenheit
double dewPointFarenheit(double farenheit, double humidity)
{
  double a = 17.271;
  double b = 237.7;
  double celsius = (farenheit - 32)/1.80;
  double temp = (a * celsius) / (b + celsius) + log(humidity*0.01);
  double TdC = (b * temp) / (a - temp);
  double TdF = (1.80 * TdC) + 32.0;
  return TdF;
}
