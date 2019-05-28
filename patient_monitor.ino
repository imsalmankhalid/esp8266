/****************************************
 * Include Libraries
 ****************************************/

#include <Wire.h>
#include <ESP8266WiFi.h>
#include "MAX30100_PulseOximeter.h"
#include <PubSubClient.h> // for MQTT communication 


/****************************************
 * Define Constants
 * Credetnials for UBI Dots (Education)
 *  username : johnsmith_p
 *  passwordl: patient1
 *  email: johnsmith@maillink.live
 *  smtp2go user: patientmonitor121@gmail.com pass: asd1ASD!
 *  patientmonitor121@gmail.com pass: asd1ASD!
 *  IFFT user patientmonitor121@gmail.com pass asd1ASD!
 ****************************************/
#define TOKEN                   "A1E-VnTOfvYuCrPwrVq4s7jDKZMS1j2XB8" // Your Ubidots TOKEN
#define REPORTING_PERIOD_MS     1000
#define MQTT_PORT               1883
#define _server                 "things.ubidots.com"
#define MAX_VALUES              3
#define PAYLOAD_SIZE            1000
#define FIRST_PART_TOPIC        "/v1.6/devices/"
#define _clientName             "Patient"
#define STASSID                 "test"        //WiFI Name
#define STAPSK                  "qwertyuiop"    //Password

/****************************************
 * Email Settings
 ****************************************/

char server[] = "mail.smtp2go.com";   // mail server
int port =      2525;                      // your outgoing port
char user[] =   "cGF0aWVudG1vbml0b3IxMjFAZ21haWwuY29t";  // base64, ASCII encoded user
char pass[] =   "cGVOWHhKUldLb1p0";                   // base64, ASCII encoded password
char from[] =   "patientmonitor121@gmail.com";        // your@email.com
char to[] =     "patientmonitor121@gmail.com";        // their@email.com
char myName[] =  "Patient Monitor";                   // Your name to display
char subject[] = "Patient Monitor - Critical";        // email subject
char message[] = "This message is from your Patient Monitor.\nCurrently patient's vitals are not normal!";
/*-------------------------------------*/

/****************************************
 * Notification Settings
 ****************************************/
const char* privateKey = "c3OfHAAoPDL1Hl6BsOXlw79SUWjgZeExdpeSPYwBMiJ";
String event = "bpm";
String event2 = "temp";
String value1 = "";
bool dataAvailable;
String postData;
/*-------------------------------------*/



WiFiClient client;
PubSubClient _client = PubSubClient(client); 

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

uint32_t  tsLastReport = 0;
int       bpm = 0,SpO2 = 0;
float     temp=0;
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
void WiFi_Connect()
{
    val = (Value *)malloc(MAX_VALUES*sizeof(Value));
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN,LOW);
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(STASSID);
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
    digitalWrite(LED_BUILTIN,!HIGH); 
}
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
void init_oximeter()
{
     // Initialize the PulseOximeter instance
    Serial.print("HeartBeat Sensor Initialization ");
    while(true)
    {
      if (!pox.begin()) 
      {
        Serial.println("FAILED");
        Serial.println("Restarting");
        delay(500);
        ESP.reset();
      } 
      else 
      {
        Serial.println("SUCCESS");
      }
      break;
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
    String str; int i=0;
    sprintf(topic, "%s%s", FIRST_PART_TOPIC, deviceLabel);
    sprintf(payload, "{");
    for (i = 0; i <= 2; i++) 
    {
        str = String((val+i)->_value, 2);
        sprintf(payload, "%s\"%s\": [{\"value\": %s", payload, (val+i)->_variableLabel, str.c_str());
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
void callback(char* topic, byte* payload, unsigned int length) 
{
  // Nothing to do
}

/****************************************
 * Email Functions
 ****************************************/
byte sendEmail(int bpm, int tmp, int spo2)
{
  byte thisByte = 0;
  byte respCode;

  if (client.connect(server, port) == 1) {
    Serial.println(F("connected"));
  } else {
    Serial.println(F("connection failed"));
    return 0;
  }
  if (!eRcv()) return 0;

  Serial.println(F("Sending EHLO"));
  client.println("EHLO www.example.com");
  if (!eRcv()) return 0;
  Serial.println(F("Sending auth login"));
  client.println("auth login");
  if (!eRcv()) return 0;
  Serial.println(F("Sending User"));
  client.println(user);
  if (!eRcv()) return 0;
  Serial.println(F("Sending Password"));
  client.println(pass);
  if (!eRcv()) return 0;
  Serial.println(F("Sending From"));
  client.println(String("MAIL From: ") + from);
  if (!eRcv()) return 0;
  Serial.println(F("Sending To"));
  client.println(String("RCPT To: ") + to);
  if (!eRcv()) return 0;
  Serial.println(F("Sending DATA"));
  client.println(F("DATA"));
  if (!eRcv()) return 0;
  Serial.println(F("Sending email"));
  client.println(String("To:  ") + to);
  client.println(String("From: ") + myName + " " + from );
  client.println(String("Subject: ") + subject + "\r\n");
  client.println(message);
  client.print("BPM :"); client.println(bpm);
  client.print("SPO2 :"); client.println(spo2);
  client.print("Temperature :"); client.println(tmp);
  
  client.println(F("."));
  if (!eRcv()) return 0;
  Serial.println(F("Sending QUIT"));
  client.println(F("QUIT"));
  if (!eRcv()) return 0;
  client.stop();
  Serial.println(F("disconnected"));
  return 1;
}

byte eRcv()
{
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while (!client.available()) {
    delay(1);
    loopCount++;
    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }

  respCode = client.peek();
  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }

  if (respCode >= '4') return 0;
  return 1;
}
/****************************************
 * Notification Functions
 ****************************************/
void post(int value, int bpm)
{
  value1 = value;
  compileData();
  Serial.print("Sending Notification for ");
  if (client.connect("maker.ifttt.com", 80))
  {
     client.print("POST /trigger/");
    if(bpm == 1)
    {
      client.print("bpm");
      Serial.println("bpm");
    }
    else
    {
      client.print("temp");
      Serial.println("temperature");
    }
      client.print("/with/key/");
      client.print(privateKey);
      client.println(" HTTP/1.1");
  
    client.println("Host: maker.ifttt.com");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    if (dataAvailable)
    { // append json values if available
      client.println("Content-Type: application/json");
      client.print("Content-Length: ");
      client.println(postData.length());
      client.println();
      client.println(postData);
    }
    else
      client.println();
  }
  else
    Serial.println("Failed to connect, going back to sleep");
  
}

void compileData()
{
  if (value1 != "" )
  {
    dataAvailable = true;
    bool valueEntered = false;
    postData = "{";
    if (value1 != "")
    {
      postData.concat("\"value1\":\"");
      postData.concat(value1);
      valueEntered = true;
    }
    postData.concat("\"}");
  }
  else dataAvailable = false;
}

 
/****************************************
 * Main Functions
 ****************************************/
void setup()
{
    Serial.begin(115200);
    WiFi_Connect();
    
    _client.setServer(_server, MQTT_PORT);
    _client.setCallback(callback);
    client_connect();
    delay(500);
    init_oximeter();
}
int count = 0;
void loop()
{

  if(WiFi.status() != WL_CONNECTED)
  {
    WiFi_Connect();
    if(WiFi.status() == WL_CONNECTED) 
    {
      client_connect();
      delay(500);
      init_oximeter();
    }
  }

  digitalWrite(LED_BUILTIN,led_state); 

    // Make sure to call update as fast as possible
    pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) 
    {
        bpm = pox.getHeartRate();
        SpO2 = pox.getSpO2();
        temp = get_temperature();
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("  bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.print("  Body Temperature: ");
        Serial.print(temp);
        Serial.println(" °C");
        add_val("BPM",bpm);
        add_val("Temp",temp);
        add_val("SpO2",SpO2);
        Publish_val("patient_vitals");
        tsLastReport = millis();
        led_state = !led_state;
        if((bpm > 10 && bpm < 50) || (bpm > 110))
        {
          count++;
          Serial.print("Critical vitals count: ");
          Serial.println(count);
          if(count > 10)
          {
            sendEmail(bpm,temp,SpO2);
            post(bpm,1);
            post(temp,2);
            count = 0;
          }
        }
    }
}
