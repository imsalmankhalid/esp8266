#include <SoftwareSerial.h>
SoftwareSerial s(D6,D5);
#include <ArduinoJson.h>
/****************************************
 * Include Libraries
 ****************************************/
#include "UbidotsESPMQTT.h"

/****************************************
 * Define Constants
 ****************************************/
#define TOKEN "A1E-hwuVdMNpoJnMtKwi0PqUL79UtHPlDo" // Your Ubidots TOKEN
#define WIFINAME "test" //Your SSID
#define WIFIPASS "qwertyuiop" // Your Wifi Pass
Ubidots client(TOKEN); 
bool state = 1;
void callback(char* topic, byte* payload, unsigned int length) {

}
void setup() {
  // Initialize Serial port
  Serial.begin(115200);
  s.begin(115200);
  client.wifiConnection(WIFINAME, WIFIPASS);
  client.begin(callback);
  pinMode(LED_BUILTIN, OUTPUT);
}


void loop() {

  if(WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN,!HIGH);
  }
  else
  {
   digitalWrite(LED_BUILTIN,!LOW); 
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
  }
if(!client.connected()){
  client.reconnect();
}
else{
  StaticJsonBuffer<100> jsonBuffer;
  if(s.available()){
    JsonObject& root = jsonBuffer.parseObject(s);
  
    if (root == JsonObject::invalid())
    {
      Serial.println("invalid");
      return;
    }
    //Print the data in the serial monitor
    Serial.println("");
    Serial.print("Heart Rate:  ");
    int data1=root["temp"];
    Serial.println(data1);
    Serial.print("SpO2    ");
    int data2=root["hum"];
    Serial.println(data2);
    Serial.println("");
    Serial.println("---------------------xxxxx--------------------");
   Serial.println("");
     client.add("bpm", data1);
    client.add("SpO2", data2);
    client.ubidotsPublish("source2");
    state=1;
  }
  else
  {
    if(state==1)
    {
      Serial.println("waiting for data");
      state=0;
    }
  }
  }
}
