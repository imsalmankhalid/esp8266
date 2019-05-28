/****************************************
 * Include Libraries
 ****************************************/
#include "UbidotsESPMQTT.h"
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <timer.h>
/****************************************
 * Define Constants
 ****************************************/
#define TOKEN "A1E-hwuVdMNpoJnMtKwi0PqUL79UtHPlDo" // Your Ubidots TOKEN
#define WIFINAME "PTCLWiFi" //Your SSID
#define WIFIPASS "03334624342s" // Your Wifi Pass
auto timer = timer_create_default(); // create a timer with default settings
Ubidots client(TOKEN);

#define REPORTING_PERIOD_MS     1000

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

int tsLastReport = 0, bpm=0,spo2=0;

/****************************************
 * Auxiliar Functions
 ****************************************/


bool toggle_led(void *) {
          Serial.print("Heart rate:");
        Serial.print(bpm);
        Serial.print("bpm / SpO2:");
        Serial.print(spo2);
        Serial.println("%");
      if(!client.connected()){
      client.reconnect();
      }
//   client.add("BPM",bpm); //Insert your variable Labels and the value to be sent
  // client.add("SpO2", spo2); //Insert your variable Labels and the value to be sent
  // client.sendAll(true);

   client.add("v0", bpm);
  client.ubidotsPublish("source1");
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

 // client.setDebug(false); // Pass a true or false bool value to activate debug messages
  client.wifiConnection(WIFINAME, WIFIPASS);
client.begin(callback);
        Serial.print("Initializing pulse oximeter..");
      if (!pox.begin(PULSEOXIMETER_DEBUGGINGMODE_RAW_VALUES)) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
      timer.every(5000, toggle_led);

  }

void loop() {
  // put your main code here, to run repeatedly:

  // Publish values to 2 different data sources
    pox.update();
    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
   if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      bpm = pox.getHeartRate();
      spo2 = pox.getSpO2();
//        Serial.print("Heart rate:");
//        Serial.print(bpm);
//        Serial.print("bpm / SpO2:");
//        Serial.print(spo2);
//        Serial.println("%");
//        Serial.println("Heap");
//        Serial.println(ESP.getFreeHeap());
        tsLastReport = millis();
    }

   timer.tick(); // tick the timer
  }
