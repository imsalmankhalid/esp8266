#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef STASSID
#define STASSID "dlink"
#define STAPSK  "password"
#endif

#define DOOR_GPIO 5
#define LED_GPIO  LED_BUILTIN

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

//Check if header is present and correct
bool is_authenticated() {
  Serial.println("Enter is_authenticated");
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentication Successful");
      return true;
    }
  }
  Serial.println("Authentication Failed");
  return false;
}

//login page, also called for disconnect
void handleLogin() {
  String msg;
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
    server.send(301);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")) {
    if (server.arg("USERNAME") == "admin" &&  server.arg("PASSWORD") == "admin") {
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
      server.send(301);
      Serial.println("Log in Successful");
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
  }
  String content = "<html><head>";
  content += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  content += "<style>body {font-family: Arial, Helvetica, sans-serif;}form {border: 3px solid #f1f1f1;}";
  content += ".login-card {padding: 40px;width: 274px;background-color: #F7F7F7;margin: 0 auto 10px;border-radius: 2px;box-shadow: 0px 2px 2px rgba(0, 0, 0, 0.3);overflow: hidden;}";
  content += "input[type=text], input[type=password] {  width: 100%;  padding: 12px 20px;  margin: 8px 0;  display: inline-block;  border: 1px solid #ccc;  box-sizing: border-box;}";
  content += "button {  background-color: #4CAF50;  color: white;  padding: 14px 20px;  margin: 8px 0;  border: none;  cursor: pointer;  width: 100%;}";
  content += "button:hover {  opacity: 0.8;}";
  content += ".container {  padding: 16px;}";
  content += "span.psw {  float: right;  padding-top: 16px;}";
  content += "}";
  content += "</style></head>";
  content += "<body><div class=\"login-card\"><form action='/login' method='POST'><br>";
  content += "<h2 align=\"center\">Login Form</h2>";
  content += "    <label for=\"uname\"><b>Username</b></label>";
  content += "<input type=\"text\" placeholder=\"Enter Username\" name=\"USERNAME\" required>";
  content += "    <label for=\"psw\"><b>Password</b></label>";
  content += "<input type=\"password\" placeholder=\"Enter Password\" name=\"PASSWORD\" required>";
  content += "<button type='submit' name='SUBMIT' value='Submit'>Login</button></form>" + msg + "<br>";
  content += "</div></body></html>";
  server.send(200, "text/html", content);
}

//root page can be accessed only if authentication is ok
void handleRoot() {
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authenticated()) {
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }
String content = "<html><head>";
  content += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  content += "<style>body {font-family: Arial, Helvetica, sans-serif;}";
  content += ".login-card {padding: 40px;width: 300px;background-color: #F7F7F7;margin: 0 auto 10px;border-radius: 2px;box-shadow: 0px 2px 2px rgba(0, 0, 0, 0.3);overflow: hidden;}";
  content += "input[type=text], input[type=password] {  width: 100%;  padding: 12px 20px;  margin: 8px 0;  display: inline-block;  border: 1px solid #ccc;  box-sizing: border-box;}";
  content += "button {  background-color: #4CAF50;  color: white;  padding: 14px 20px;  margin: 8px 0;  border: none;  cursor: pointer;  width: 100%;}";
  content += "button:hover {  opacity: 0.8;}";
  content += ".container {  padding: 16px;}";
  content += "span.psw {  float: right;  padding-top: 16px;}";
  content += "}";
  content += "</style></head>";
  content += "<body><div class=\"login-card\"><br>";
  content += "</style></head>";
  content += "<body><div class='login-card'>";
  content += "<h2 align='center'>Control System</h2>";
  content += "<label><b>Door  <p><a href='/door_open'><button>Open </button></a></p></b></label>";
  content += "<p><a href='/door_close'><button>Close</button></a></p>";
  content += "<label><b>Light  <p><a href='/light_on'><button>On </button></a></p></b></label>";
  content += "<p><a href='/light_off'><button>Off</button></a></p>";
  content += "<h4 align='center'><a href='/login?DISCONNECT=YES'>Logout</a></h2>";
  content += "</div></body></html>";
  server.send(200, "text/html", content);
}

//no need authentication
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
void handleDoor_open() {
  digitalWrite(DOOR_GPIO, HIGH);
  Serial.println("Door Open");
   server.sendHeader("Location", "/");
   server.sendHeader("Cache-Control", "no-cache");
   server.send(301);
}

void handleDoor_close() {
  digitalWrite(DOOR_GPIO, LOW);
  Serial.println("Door Close");
  server.sendHeader("Location", "/");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(301);
}

void handleLight_on() {
  digitalWrite(LED_GPIO, LOW);
  Serial.println("Light on");
  server.sendHeader("Location", "/");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(301);
}

void handleLight_off() {
  digitalWrite(LED_GPIO, HIGH);
  Serial.println("Light off");
  server.sendHeader("Location", "/");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(301);
}

void setup(void) {
  pinMode(LED_GPIO, OUTPUT);
  pinMode(DOOR_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, HIGH);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/door_close", handleDoor_close);
  server.on("/door_open", handleDoor_open);
  server.on("/light_off", handleLight_off);
  server.on("/light_on", handleLight_on);

  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
