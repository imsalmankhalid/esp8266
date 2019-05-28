/*
Arduino-MAX30100 oximetry / heart rate integrated sensor library
Copyright (C) 2016  OXullo Intersecans <x@brainrapers.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Wire.h>
#include <WiFi.h>
#include "MAX30100_PulseOximeter.h"
//#include "UbidotsESPMQTT.h"

/****************************************
 * Define Constants
 ****************************************/
#define TOKEN "A1E-hwuVdMNpoJnMtKwi0PqUL79UtHPlDo" // Your Ubidots TOKEN
#define WIFINAME "test" //Your SSID
#define WIFIPASS "qwertyuiop" // Your Wifi Pass
//Ubidots client(TOKEN); 
bool state = 1;

#define REPORTING_PERIOD_MS     1000

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

uint32_t tsLastReport = 0;
void callback(char* topic, byte* payload, unsigned int length) {}
// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("Beat!");
}

void setup()
{
    Serial.begin(115200);

    Serial.print("Initializing pulse oximeter..");

    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pox.begin()) 
    {
        Serial.println("FAILED");
    } 
    else {
        Serial.println("SUCCESS");
    }
  Serial.begin(115200);
//  client.wifiConnection(WIFINAME, WIFIPASS);
//  client.begin(callback);
WiFi.begin(WIFINAME, WIFIPASS);
  pinMode(LED_BUILTIN, OUTPUT);
    // The default current for the IR LED is 50mA and it could be changed
    //   by uncommenting the following line. Check MAX30100_Registers.h for all the
    //   available options.
    // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);
}
int bpm,SpO2;
void loop()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN,!HIGH);
  }
  else
  {
   digitalWrite(LED_BUILTIN,!LOW); 
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    
   }
}
while(1){ Serial.print("*"); }
//if(!client.connected()){
//  client.reconnect();
//}
    // Make sure to call update as fast as possible
    pox.update();

    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        bpm = pox.getHeartRate();
        SpO2 = pox.getSpO2();
        Serial.print("Heart rate:");
        Serial.print(bpm);
        Serial.print("bpm / SpO2:");
        Serial.print(SpO2);
        Serial.println("%");

        tsLastReport = millis();
//        client.add("bpm", bpm);
//        client.add("SpO2", SpO2);
//        client.ubidotsPublish("source2");
    }
}
