

// Wifi Settings
const char* SSID = "test";        // your wifi network
const char* PASS = "03334624342s";       // your wifi password

// Email Settings
char server[] = "mail.smtp2go.com";   // mail.yourMailServer.com
int port =      2525;                      // your outgoing port; my server uses 25
char user[] =   "cGF0aWVudG1vbml0b3IxMjFAZ21haWwuY29t";                  // base64, ASCII encoded user
char pass[] =   "cGVOWHhKUldLb1p0";                  // base64, ASCII encoded password
char from[] =   "patientmonitor121@gmail.com";        // your@email.com
char to[] =     "patientmonitor121@gmail.com";       // their@email.com
char myName[] =  "Your Name";            // Your name to display
char subject[] = "esp8266 Test";         // email subject
char message[] = "This message is from your esp8266.\nIt works!";
//-------------------------------------



#include <ESP8266WiFi.h>
WiFiClient client;
void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println("");
  Serial.println("");
  Serial.print("Connecting To ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(0, INPUT_PULLUP);
}

long timer = millis();

void loop()
{

    byte ret = sendEmail();
    delay(5000);

}

byte sendEmail()
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
