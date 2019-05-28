#include <Arduino.h>
#include <ESP8266WiFi.h>
const char* ssid     = "test";
const char* password = "03334624342s";

    WiFiClient client;
    const char* privateKey = "c3OfHAAoPDL1Hl6BsOXlw79SUWjgZeExdpeSPYwBMiJ";
    String event = "bpm";
    String value1 = "";
    bool dataAvailable;
    String postData;



void post(String value)
{
  value1 = value;
  compileData();
  if (client.connect("maker.ifttt.com", 80))
  {
    client.print("POST /trigger/");
    client.print(event);
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

void initWifi() {
  Serial.print("Connecting to: "); 
  Serial.print(ssid);
  WiFi.begin(ssid, password);  

  int timeout = 10 * 4; // 10 seconds
  while(WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");

  if(WiFi.status() != WL_CONNECTED) {
     Serial.println("Failed to connect, going back to sleep");
  }

  Serial.print("WiFi connected in: "); 
  Serial.print(millis());
  Serial.print(", IP address: "); 
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200); 

  initWifi();

 Serial.println("Sending Notification");
 post("66");
 Serial.println("Sent");
}
void loop()
{
  
}
