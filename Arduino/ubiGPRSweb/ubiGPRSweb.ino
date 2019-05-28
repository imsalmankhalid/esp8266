/**************************************************************
 *
 * This sketch connects to a website and downloads a page.
 * It can be used to perform HTTP/RESTful API calls.
 *
 * TinyGSM Getting Started guide:
 *   http://tiny.cc/tiny-gsm-readme
 *
 **************************************************************/

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_ESP8266

// Increase RX buffer if needed
//#define TINY_GSM_RX_BUFFER 512

#include <TinyGsmClient.h>
// Uncomment this if you want to see all AT commands
//#define DUMP_AT_COMMANDS

// Uncomment this if you want to use SSL
//#define USE_SSL

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// or Software Serial on Uno, Nano
//#define SerialAT Serial1

// or Software Serial on Uno, Nano
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(3,2); // RX, TX

// Your GPRS credentials
// Leave empty, if missing user or pass
//Network details
const char apn[]  = "zongwap";
const char user[] = "";
const char pass[] = "";

// Server details
const char server[] = "things.ubidots.com";
const char resource[] = "/api/v1.6/devices/temp/new-variable?token=A1E-VnTOfvYuCrPwrVq4s7jDKZMS1j2XB8";

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

#ifdef USE_SSL
  TinyGsmClientSecure client(modem);
  const int  port = 443;
#else
  TinyGsmClient client(modem);
  const int  port = 80;
#endif


float parseUbiResponse(char* data){
  // parses the answer

  char* parsed = (char *) malloc(sizeof(char) * 20);
  char* dst = (char *) malloc(sizeof(char) * 20);
  int len = strlen(data); // Length of the answer char array from the server

  for (int i = 0; i < len - 2; i++) {
    if ((data[i] == '\r') && (data[i+1] == '\n') && (data[i+2] == '\r') && (data[i+3] == '\n')) {
        strncpy(parsed, data+i+4, 20);  // Copies the result to the parsed
        parsed[20] = '\0';
        break;
    }
  }

  /* Extracts the the value */

  uint8_t index = 0;

  // Creates pointers to split the value
  char *pch = strchr(parsed, '\n');
  char *pch2 = strchr(pch+1, '\n');
  index = (int)(pch2 - pch - 1);

  memcpy(dst, pch + 1, index);
  dst[strlen(dst) - 1] = '\0';

  float result = atof(dst);
  free(parsed);
  free(dst);

  return result;
}

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(9600);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("Initializing modem..."));
  modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print(F("Modem: "));
  SerialMon.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");
}

void loop() {
  SerialMon.print(F("Waiting for network..."));
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  SerialMon.print(F("Connecting to "));
  SerialMon.print(server);
  if (!client.connect(server, port)) {
    SerialMon.println(" fail");
    delay(1000);
    return;
  }
  SerialMon.println(" OK");

  // Make a HTTP GET request:
  client.print(String("GET ") + resource + " HTTP/1.0\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print("Connection: close\r\n\r\n");

unsigned long timeout = millis();
int i=0;
char* response = (char *) malloc(sizeof(char) * 1000);
  while (client.connected() && millis() - timeout < 10000L) {
    // Print available data
    while (client.available()) {
      char c = client.read();
      response[i++] = c;
      SerialMon.print(c);
      timeout = millis();
    }
}
  response[i] = '\0';
  SerialMon.println(response);

  // Shutdown
float result = parseUbiResponse(response);
Serial.println("result:");
  Serial.println(result);
  free(response);
  client.stop();
  SerialMon.println(F("Server disconnected"));

  modem.gprsDisconnect();
  SerialMon.println(F("GPRS disconnected"));

}
