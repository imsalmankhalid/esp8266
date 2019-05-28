/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo and Micro support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.

 */
#include <SoftwareSerial.h>
#include <EEPROM.h>


SoftwareSerial mySerial(6,5); // RX, TX
int data[15], i=0;
String code = "";           //initialize the output string
String stored_code = "",relay_set="off";           //initialize the output string
boolean endbit = 0;            //a flag to mark 0D received
char temp;
const int relay =  8;// the number of the Relay pin
uint8_t relay_st = 0;
void writeString(char add,String data)
{
  int _size = data.length();
  int i;
  for(i=0;i<_size;i++)
  {
    EEPROM.write(add+i,data[i]);
  }
  EEPROM.write(add+_size,'\0');   //Add termination null character for String Data
}

String read_String(char add)
{
  int i;
  char data[100]; //Max 100 Bytes
  int len=0;
  unsigned char k;
  k=EEPROM.read(add);
  while(k != '\0' && len<500)   //Read until null character
  {    
    k=EEPROM.read(add+len);
    data[len]=k;
    len++;
  }
  data[len]='\0';
    return String(data);
}

void setup() {
  Serial.begin(9600);       //initialize the Serial port
  mySerial.begin(9600);       //initialize the Serial port
    //configure pin 2 as an input and enable the internal pull-up resistor
  pinMode(2, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  pinMode(relay, OUTPUT);
  stored_code = read_String(10);
  Serial.print("Stored code");
  Serial.println(stored_code);
  relay_set = read_String(50);  
  if (relay_set.equals("on"))
  {
    relay_st = LOW;
    Serial.println("Default relay setting: ON");
  }
  else
  {
    relay_st = HIGH;
    Serial.println("Default relay setting: OFF");
  }
  digitalWrite(relay, relay_st);
}

void loop() {

int buttonState = digitalRead(2);
  
  if (mySerial.available() > 0)     {
    temp = char( mySerial.read());    //read the input data
    code += temp;
    Serial.print(temp);
  }
  if (temp == 0x0D){           // Or temp == '\r'
    if(buttonState == LOW)
    {
      writeString(10,code);
      Serial.print("New code stored: ");
      Serial.println(code);
      stored_code = read_String(10);
    }
    else
    {
      Serial.println("High");
    }
    if(stored_code != "")
    {
      if(code.equals(stored_code))
      {
        Serial.print("\nData");
        Serial.println(code);
        digitalWrite(relay, !relay_st);
      }
      else if (code.equals("RELAYON\r"))
      {
        Serial.println("Setting default Relay to ON");
        writeString(50,"on");
        relay_st = LOW;
        digitalWrite(relay, relay_st);
      }
      else if (code.equals("RELAYOFF\r"))
      {
        Serial.println("Setting default Relay to OFF");
        writeString(50,"off");
        relay_st = HIGH;
        digitalWrite(relay, relay_st);
      } 
      else
      {
        digitalWrite(relay, relay_st);
      }
    }
    else
    {
      Serial.println("No Code stored in memory");
    }
    code = "";
    endbit = 0;
    temp = 0;
  }
  digitalWrite(13, !buttonState);
}
