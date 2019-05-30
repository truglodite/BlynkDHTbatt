# BlynkDHTbatts
for: ESP8266 + DHTXX + deepsleep + battery monitor + OTA + Blynk
by: Truglodite

## General Operation
Deepsleeps, wakes after "sleepMicros" (10min default), and sends Temperature (F), Humidity (%), Heat Index (F), Wifi RSSI (dBm), and battery level (V) to a Blynk server. When battery voltage drops below "vbattLow", data and a notification are sent every 70min. When the battery voltage drops below a set critical threshold, data and a notification are sent, and the esp goes to deepsleep until battery is removed/replaced (deepsleep(0)).

## OTA Firmware Updates
If the app Firmware button is turned on and battery is not low, when the esp next awakens it will send a notification and stay on to wait for an ArduinoOTA upload. Do not turn the firmware button off while uploading. You may turn off the firmware button to resume normal operation after upload/reboot and second notification is sent. If you accidentally hit the firmware button, you may turn it off before uploading... it will send current data and resume normal operation. If the FW button is on longer than "otaTimeout", it will restart to send a reminder notification. If b

## Notes on Battery Life
Power to the DHT sensor is controlled by io5. The DHT is turned off while asleep to save battery. Note that the DHT is an appreciable capacitive load, so low quiescent LDO's may have trouble with voltage spike resets. A somewhat large bypass cap may be required for stability; note that some candidate capacitors (ie: electrolytic) have very large leakage current, which can drastically reduce battery life.

Note that you must add a notification widget to the Blynk app  for low battery and OTA ready notifications to work. Additional notifications can be added for convenience (ie a 'freezing' alert).

Maximize deepsleep time (sleepMinutes) to preserve battery. My experiments lean towards Blynk.begin() as the faster connect. Eliminating unnecessary peripherals (ie LEDs and USB/UART chips), and using low quiescent LDO's (ADP160AU is good) and low leakage capacitors can achieve extreme battery life. For example a 10min sleep time, static IP, and 3000mAh lion can last nearly one year.

## Blynk V-Pins
Vpin | Data Output (all user optional... no BLYNK_WRITE()'s here)
--- | ---------------------
V0 | Temperature [F]
V1 | Relative Humidity [%]
V2 | Heat Index [F]
V3 | Wifi RSSI [dBm] *with 'extraOutput'*
V4 | Battery Voltage [V]
V5 | Firmware Upload Button
V6 | Last awake time [msec]
V7 | Dew Point [F]

## ESP8266 Pins
ESPpin | Description
------ | -------------------
io4 | DHT sensor data pin
io5 | DHT Vcc pin
A0 | Battery voltage divider output (default 330k/100k with 1s Li-Ion)
EN | 10k High
io2 | 10k High
io15 | 10k Low
io16 | RST - 220ohm resistor link for self-wake from deepsleep
