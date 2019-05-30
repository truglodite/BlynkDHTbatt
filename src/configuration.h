// configuration.h
// by: Truglodite
// 5-30-2019
// User configuration for blynkDHTbatts.ino
#define batteryMonitor                            // Comment out if plugged in; disables adc stuff

#ifndef privacy
char auth[] = "blynkAuthToken";                   // Blynk App auth token
char ssid[] = "WifiSSID";                         // Wifi SSID
char pass[]= "WifiPassword";                      // Wifi WPA2 password
//IPAddress staticIP          (192,168,1,3);      // Static local IP (setup your router accordingly)
//byte mac[] = {0xDE,0xAD,0xBE,0xEF,0xFE,0xED};   // Wifi MAC
const char* hostName =      "esp-myNode";         // OTA hostname, default "esp8266-[ChipID]"
const char* update_username = "username";         // OTA username
const char* update_path =   "/firmware";          // OTA webserver update directory
const char* otaPassword =   "password";           // OTA password
char notifyLowBatt[] =      "myNode: Battery Low";// Low battery notification text
char notifyOTAreadyX[] =    "OTA Waiting\nhttp://";// OTA ready notification text
char notifyDHTfail[] =      "myNode: DHT read error";// Failed DHT read notification text
#ifdef batteryMonitor
  float vbattRatio =        4.237;                // Input voltage at max 1.0V output (approx. 4.0-4.4 for
                                                  // 330k-100k with one L-ion cell, calibrate for accuracy)
#endif
#endif

//#define extraOutput                             // Uncomment to enable RSSI & millis() output.

int retriesMax = 5;                               // Number of failed DHT reads before rebooting
#ifdef batteryMonitor
  float vbattLow =          3.75;                  // battery voltage for low voltage mode
  float vbattCrit =         3.6;                  // battery voltage for "all deepsleep" mode
#endif
#define dhtPin              4                     // DHT sensor data pin (default io4)
#define dhtPowerPin         5                     // DHT sensor power pin (default io5)
#define dhtType             DHT22                 // DHT22, DHT11, etc...
#define tempVpin            V0                    // Temperature [F]
#define humidVpin           V1                    // Relative Humidity [%]
#define dewpointVpin        V7                    // Dewpoint [F]
#define indexVpin           V2                    // Heat Index [F]
#define rssiVpin            V3                    // RSSI [dBm]
#define battVpin            V4                    // Battery voltage [V]
#define firmwareVpin        V5                    // Firmware Upload Button, 0= normal, 1= stay awake for OTA
#define millisVpin          V6                    // Last awake time [msec]
uint32_t sleepMicros =      600000000;            // uSec to sleep between pubs (60000000 = 10min default)
uint32_t longSleepMicros =  4294967295;           // uSec to sleep between low battery pubs (max ~70min)
unsigned long dhtTimeout =  2000;                 // mSec to give DHT to read before deepsleep (may reduce excessive 'NAN' outputs)
unsigned long otaTimeout =  300000;               // mSec to wait for OTA upload before reboot
