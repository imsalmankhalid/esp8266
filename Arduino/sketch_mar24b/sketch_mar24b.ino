#include <SoftwareSerial.h>
#include <ArduinoJson.h>
SoftwareSerial s(5,6);

 
void setup() {
  s.begin(115200);
  s.begin(115200);
}
StaticJsonBuffer<1000> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();
void loop() {
 int d1,d2;
 d1 = random(100); d2=random(100);
  root["temp"] = d1;
  root["hum"] = d2;

if(s.available()>0)
{
 root.printTo(s);
}
Serial.print(d1); Serial.println(d2);
 delay(1000);
}
