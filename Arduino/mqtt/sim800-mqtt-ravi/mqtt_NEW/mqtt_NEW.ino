#define TINY_GSM_MODEM_SIM800

#include <SoftwareSerial.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>

SoftwareSerial SerialAT(3,2); // RX, TX

//Network details
const char apn[]  = "zongwap";
const char user[] = "";
const char pass[] = "";

// MQTT details
const char* broker = "things.ubidots.com";
const char* topicOut = "shahrulnizam/out";
const char* topicIn = "shahrulnizam/in";

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

void setup()
{
  Serial.begin(9600);
  SerialAT.begin(9600);
  
  Serial.println("System start.");
  //modem.restart();
  Serial.println("Modem: " + modem.getModemInfo());
  Serial.println("Searching for telco provider.");
  while(!modem.waitForNetwork())
  {
    Serial.println("fail");
    delay(500);
  }
  Serial.println("Connected to telco.");
  Serial.println("Signal Quality: " + String(modem.getSignalQuality()));

  Serial.println("Connecting to GPRS network.");
  while (!modem.gprsConnect(apn, user, pass))
  {
    Serial.println("fail");
    delay(500);
  }
  Serial.println("Connected to GPRS: " + String(apn));
  
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);
  Serial.println("Connecting to MQTT Broker: " + String(broker));
  while(mqttConnect()==false) continue;
  Serial.println();
}

void loop()
{
  if(Serial.available())
  {
    delay(10);
    String message="";
    while(Serial.available()) message+=(char)Serial.read();
    mqtt.publish(topicOut, message.c_str());
  }
  
  if(mqtt.connected())
  {
    mqtt.loop();
  }
}

boolean mqttConnect()
{
  if(!mqtt.connect("GsmClientTest"))
  {
    Serial.print(".");
    return false;
  }
  Serial.println("Connected to broker.");
  mqtt.subscribe(topicIn);
  return mqtt.connected();
}

void mqttCallback(char* topic, byte* payload, unsigned int len)
{
  Serial.print("Message receive: ");
  Serial.write(payload, len);
  Serial.println();
}
