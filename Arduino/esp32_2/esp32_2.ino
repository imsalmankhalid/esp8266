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
#include <ESP8266WiFi.h>
#include "MAX30100_PulseOximeter.h"

#include <PubSubClient.h>

#define TOKEN "A1E-hwuVdMNpoJnMtKwi0PqUL79UtHPlDo" // Your Ubidots TOKEN

#define REPORTING_PERIOD_MS     1000
#define MQTT_PORT 1883
#define _server "things.ubidots.com"
#define MAX_VALUES 3
#define FIRST_PART_TOPIC "/v1.6/devices/"
#define _clientName "ESP8266"

WiFiClient espClient;
PubSubClient _client = PubSubClient(espClient); 

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

uint32_t tsLastReport = 0;
int bpm,SpO2;
uint8_t currentValue=0;

typedef struct Value {
    char  *_variableLabel;
    float _value;
    char *_context;
    char *_timestamp;
} Value;
Value * val;

void callback(char* topic, byte* payload, unsigned int length) {

}
void setup()
{
    Serial.begin(115200);
    val = (Value *)malloc(MAX_VALUES*sizeof(Value));
    
    
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println("test");
   // WiFi.setAutoConnect (true);
   // WiFi.setAutoReconnect (true);
    WiFi.begin("PTCLWiFi", "03334624342s");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); 
    Serial.print("Initializing pulse oximeter..");

    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip

    // The default current for the IR LED is 50mA and it could be changed
    //   by uncommenting the following line. Check MAX30100_Registers.h for all the
    //   available options.
    // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

    _client.setServer(_server, MQTT_PORT);
    _client.setCallback(callback);
    client_connect();
      if (!pox.begin()) {
      Serial.println("FAILED");
      for(;;);
  } else {
      Serial.println("SUCCESS");
  }
}

void loop()
{

  if(WiFi.status() != WL_CONNECTED){
     WiFi.begin("test", "qwertyuiop");
    while (true){
      if(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      else{
        client_connect();
              if (!pox.begin()) {
              Serial.println("FAILED");
              for(;;);
          } else {
              Serial.println("SUCCESS");
          }
          break;
      }
      }
    }

  
    // Make sure to call update as fast as possible
    

    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        bpm = pox.getHeartRate();
        SpO2 = pox.getSpO2();
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");
        add_val("BPM",bpm);
        add_val("SpO2",SpO2);
     //   Serial.println("Step1");
        Publish_val("esp32");
        tsLastReport = millis();
    }
}


/******************/
void client_connect()
{
  while(!_client.connected())
  if (!_client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (_client.connect(_clientName, TOKEN, NULL)) {
            Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(_client.state());
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
  }
}

void add_val(char* variableLabel, float value) {
    (val+currentValue)->_variableLabel = variableLabel;
    (val+currentValue)->_value = value;
    (val+currentValue)->_context = NULL;
    (val+currentValue)->_timestamp = NULL;
    currentValue++;
}

bool Publish_val(char *deviceLabel) {
    char topic[150];
    char payload[500];
    String str;
    sprintf(topic, "%s%s", FIRST_PART_TOPIC, deviceLabel);
    sprintf(payload, "{");
   // Serial.println("Step2");
    for (int i = 0; i <= 2; ) {
        str = String((val+i)->_value, 2);
        sprintf(payload, "%s\"%s\": [{\"value\": %s", payload, (val+i)->_variableLabel, str.c_str());
  //      Serial.println("Step3");
        i++;
        if (i >= 2) {
            sprintf(payload, "%s}]}", payload);
            break;
        } else {
            sprintf(payload, "%s}], ", payload);
        }
    }
    if (!true){
        Serial.println("publishing to TOPIC: ");
        Serial.println(topic);
        Serial.print("JSON dict: ");
        Serial.println(payload);
    }
    currentValue = 0;
    return _client.publish(topic, payload, 512);
}

int I2C_ClearBus() {
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif
  pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);

  delay(2500);  // Wait 2.5 secs. This is strictly only necessary on the first power
  // up of the DS3231 module to allow it to initialize properly,
  // but is also assists in reliable programming of FioV3 boards as it gives the
  // IDE a chance to start uploaded the program
  // before existing sketch confuses the IDE by sending Serial data.

  boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
  if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master. 
    return 1; //I2C bus error. Could not clear SCL clock line held low
  }

  boolean SDA_LOW = (digitalRead(SDA) == LOW);  // vi. Check SDA input.
  int clockCount = 20; // > 2x9 clock

  while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
    clockCount--;
  // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
    pinMode(SCL, INPUT); // release SCL pullup so that when made output it will be LOW
    pinMode(SCL, OUTPUT); // then clock SCL Low
    delayMicroseconds(10); //  for >5uS
    pinMode(SCL, INPUT); // release SCL LOW
    pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5uS
    // The >5uS is so that even the slowest I2C devices are handled.
    SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
    int counter = 20;
    while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.
      counter--;
      delay(100);
      SCL_LOW = (digitalRead(SCL) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
    }
    SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
  }
  if (SDA_LOW) { // still low
    return 3; // I2C bus error. Could not clear. SDA data line held low
  }

  // else pull SDA line low for Start or Repeated Start
  pinMode(SDA, INPUT); // remove pullup.
  pinMode(SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5uS
  pinMode(SDA, INPUT); // remove output low
  pinMode(SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5uS
  pinMode(SDA, INPUT); // and reset pins as tri-state inputs which is the default state on reset
  pinMode(SCL, INPUT);
  return 0; // all ok
}
