/****************************************
 * Include Libraries
 ****************************************/

#include <Wire.h>
#include <ESP8266WiFi.h>
#include "MAX30100_PulseOximeter.h"
#include <PubSubClient.h>


/****************************************
 * Define Constants
 ****************************************/
#define TOKEN                   "A1E-hwuVdMNpoJnMtKwi0PqUL79UtHPlDo" // Your Ubidots TOKEN
#define REPORTING_PERIOD_MS     1000
#define MQTT_PORT               1883
#define _server                 "things.ubidots.com"
#define MAX_VALUES              3
#define PAYLOAD_SIZE            500
#define FIRST_PART_TOPIC        "/v1.6/devices/"
#define _clientName             "ESP8266"
#define STASSID                 "test"
#define STAPSK                  "qwertyuiop"

WiFiClient espClient;
PubSubClient _client = PubSubClient(espClient); 

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

uint32_t  tsLastReport = 0;
int       bpm = 0,SpO2 = 0;
uint8_t   currentValue=0, led_state=0;

typedef struct Value {
    char  *_variableLabel;
    float _value;
    char *_context;
    char *_timestamp;
} Value;
Value* val;


/****************************************
 * Auxiliar Functions
 ****************************************/
void client_connect()
{
  while(!_client.connected())
  {
    if (!_client.connected()) 
    {
      Serial.print("Attempting MQTT connection...");
      if (_client.connect(_clientName, TOKEN, NULL)) 
      {
        Serial.println("connected");
      }
      else 
      {
        Serial.print("failed, rc=");
        Serial.print(_client.state());
        Serial.println(" try again in 3 seconds");
        delay(3000);
      }
    }
  }
}

void add_val(char* variableLabel, float value) 
{
    (val+currentValue)->_variableLabel = variableLabel;
    (val+currentValue)->_value = value;
    (val+currentValue)->_context = NULL;
    (val+currentValue)->_timestamp = NULL;
    currentValue++;
}

bool Publish_val(char *deviceLabel) 
{
    char topic[150];
    char payload[PAYLOAD_SIZE];
    String str;
    sprintf(topic, "%s%s", FIRST_PART_TOPIC, deviceLabel);
    sprintf(payload, "{");
    for (int i = 0; i <= 2; ) 
    {
        str = String((val+i)->_value, 2);
        sprintf(payload, "%s\"%s\": [{\"value\": %s", payload, (val+i)->_variableLabel, str.c_str());
        i++;
        if (i >= 2) 
        {
            sprintf(payload, "%s}]}", payload);
            break;
        } 
        else 
        {
            sprintf(payload, "%s}], ", payload);
        }
    }
    if (!true)
    {
        Serial.println("publishing to TOPIC: ");
        Serial.println(topic);
        Serial.print("JSON dict: ");
        Serial.println(payload);
    }
    currentValue = 0;
    return _client.publish(topic, payload, 512);
}
float get_temperature()
{
  int analogValue = analogRead(A0);
  float millivolts = (analogValue/1024.0) * 3300; //3300 is the voltage provided by NodeMCU
  return (millivolts/10); //Temperature in milivolts
}
void callback(char* topic, byte* payload, unsigned int length) {

}
/****************************************
 * Main Functions
 ****************************************/
void setup()
{
    Serial.begin(115200);
    val = (Value *)malloc(MAX_VALUES*sizeof(Value));
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN,LOW);
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println("test");
    WiFi.setAutoConnect (true);
    WiFi.setAutoReconnect (true);
    WiFi.begin(STASSID, STAPSK);

    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); 
    Serial.print("Initializing pulse oximeter..");


    _client.setServer(_server, MQTT_PORT);
    _client.setCallback(callback);

    // Initialize the PulseOximeter instance
    while(true)
    {
      if (!pox.begin()) 
      {
        Serial.println("FAILED");
        delay(500);
      } 
      else 
      {
        Serial.println("SUCCESS");
        break;
      }
      client_connect();
    }
}

void loop()
{

  if(WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin("test", "qwertyuiop");
    while (true)
    {
      if(WiFi.status() != WL_CONNECTED) 
      {
        digitalWrite(LED_BUILTIN,!LOW); 
        delay(500);
        Serial.print(".");
      }
      else
      {
        digitalWrite(LED_BUILTIN,!HIGH);
        client_connect();
        if (!pox.begin()) 
        {
          Serial.println("FAILED");
          for(;;);
        } 
        else 
        {
          Serial.println("SUCCESS");
        }
          break;
      }
    }
  }

  digitalWrite(LED_BUILTIN,led_state); 

    // Make sure to call update as fast as possible
    pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) 
    {
        bpm = pox.getHeartRate();
        SpO2 = pox.getSpO2();
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.print("Body Temperature: ");
        Serial.print(get_temperature());
        Serial.println(" °C");
        add_val("BPM",bpm);
        add_val("SpO2",SpO2);
        Publish_val("esp32");
        tsLastReport = millis();
        led_state = !led_state;
    }
}
