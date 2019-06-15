/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>


const char* ssid     = "PTCLWiFi";
const char* password = "03334624342s";
const char* host = "http-test.000webhostapp.com";


void setup() {
  Serial.begin(115200);
  delay(10);
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  delay(10);

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/postdemo.php";
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  String PostData = "status=" + (String)random(200) + "&station=1";
  // This will send the request to the server
 client.println("POST /postdemo.php HTTP/1.1");
client.println("Host: http-test.000webhostapp.com");
client.println("Cache-Control: no-cache");
client.println("Content-Type: application/x-www-form-urlencoded");
client.print("Content-Length: ");
client.println(PostData.length());
client.println();
client.println(PostData);
String line;
int count = 0;
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    Serial.write(client.read());
  }

  Serial.println();
  Serial.println("closing connection");
  delay(20000);
}
