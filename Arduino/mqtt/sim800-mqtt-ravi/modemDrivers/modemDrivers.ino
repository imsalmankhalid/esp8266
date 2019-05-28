boolean resetModem() {
  uint8_t answer = 0;
  digitalWrite(sim800lreset, LOW);
  delay(1000);
  digitalWrite(sim800lreset, HIGH);
  // checks if the module is started
  delay(3000);
  answer = sendATcommand("AT", "OK", 2000);
  if (answer == 1)sendATcommand("AT", "OK", 2000); // turn off the echo
  else if (answer == 0)
  {
    // power on pulse
    digitalWrite(sim800lreset, LOW);
    delay(1000);
    digitalWrite(sim800lreset, HIGH);

    // waits for an answer from the module
    int trials = 0;
    while (answer == 0) {
      trials++;
      // Send AT every two seconds and wait for the answer
      answer = sendATcommand("AT", "OK", 2000);
      if (trials == 5) {
        Serial.println(F("Gsm Start Fail"));
        //break;
        return false;
      }
    }
    sendATcommand("ATE0", "OK", 2000);// turn off the echo
  }
  else if (answer == 1)return true;
}



int8_t readServerResponse(char* ATcommand, char* expected_answer1, char* expected_answer2, unsigned int timeout) {
  unsigned long nowMillis = millis();
  Serial2.println(ATcommand);
  delay(3000);
 
  if (Serial2.available()) {
    while (char(Serial.read()) != 0x24) {
      if ((millis() - nowMillis) > 2000) {
        Serial.println("NO DATA RECEIVED FROM REMOTE");
        break;
      }
    }
    nowMillis=(millis());
    while (Serial2.available()) {
      Serial.print(char(Serial2.read()));
    }
  }

}


int8_t sendATcommand2(char* ATcommand, char* expected_answer1, char* expected_answer2, unsigned int timeout) {

  uint8_t x = 0,  answer = 0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    // Initialize the string

  delay(100);

  Serial2.flush();
  Serial2.println(ATcommand);    // Send the AT command
  //if(strstr(ATcommand, "AT+CIPSEND")!=NULL) Serial2.write(0x1A);

#ifdef dbg
  Serial.println(ATcommand);    // Send the AT command
#endif

  x = 0;
  previous = millis();

  // this loop waits for the answer
  do {
    // if there are data in the UART input buffer, reads it and checks for the asnwer
    if (Serial2.available() != 0) {
      response[x] = Serial2.read();
      x++;
      // check if the desired answer 1  is in the response of the module
      if (strstr(response, expected_answer1) != NULL)
      {
        answer = 1;
        while (Serial.available()) {
          response[x] = Serial2.read();
          x++;
        }
      }
      // check if the desired answer 2 is in the response of the module
      else if (strstr(response, expected_answer2) != NULL)
      {
        answer = 2;
        while (Serial.available()) {
          response[x] = Serial2.read();
          x++;
        }
      }

    }
  }
  // Waits for the asnwer with time out
  while ((answer == 0) && ((millis() - previous) < timeout));
#ifdef dbg
  Serial.println(response);
#endif
  return answer;
}
/////////////////////


int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout) {

  uint8_t x = 0,  answer = 0;
  char response[500];
  unsigned long previous;
  char* str;
  uint8_t index = 0;

  memset(response, '\0', 100);    // Initialize the string

  delay(100);

  while ( Serial2.available() > 0) Serial2.read();   // Clean the input buffer

  Serial2.println(ATcommand);    // Send the AT command
#ifdef dbg
  Serial.println(ATcommand);    // Send the AT command
#endif


  x = 0;
  previous = millis();

  // this loop waits for the answer
  do {
    if (Serial2.available() != 0) {
      // if there are data in the UART input buffer, reads it and checks for the asnwer
      response[x] = Serial2.read();
      //Serial2.print(response[x]);
      x++;
      // check if the desired answer  is in the response of the module
      if (strstr(response, expected_answer) != NULL)
      {
        answer = 1;

      }
    }
  }
  // Waits for the asnwer with time out
  while ((answer == 0) && ((millis() - previous) < timeout));

#ifdef dbg
  Serial.println(response);    // Send the AT command
#endif
  return answer;
}





//void updateTime() {
//  sendATcommand("AT+CCLK?", "OK", 2000);
//}


int initTCP() {

  resetModem();
  sendATcommand2("ATE0", "OK", "ERROR", 2000);
  sendATcommand2("ATE0", "OK", "ERROR", 2000);

  delay(2000);


  Serial.println(F("Connecting to the network..."));

  while ( sendATcommand2("AT+CREG?", "+CREG: 0,1", "+CREG: 0,5", 1000) == 0 );
  delay(2000);


  if (sendATcommand2("AT+CIPMUX=0", "OK", "ERROR", 1000) == 1)      // Selects Single-connection mode

  {
    if (sendATcommand2("AT+CIPRXGET=2", "OK", "ERROR", 1000) == 1) { //RECEIVE DATA manually FROM THE REMOTE SERVER
      int8_t answer = 0;

      delay(1000);

      if (!(sendATcommand2("AT+CIPMODE=0", "OK", "ERROR", 1000) ))return 0;//srt non transparent mode for data sending
      delay(500);
      if (!(sendATcommand2("AT+CIPSRIP=0", "OK", "ERROR", 1000) ))return 0;//Do not show the prompt during receiving data from server
      delay(500);
      while (sendATcommand("AT+CGATT?", "+CGATT: 1", 5000) == 0 );
      delay(1000);

      // Waits for status IP INITIAL
      while (sendATcommand("AT+CIPSTATUS", "INITIAL", 5000) == 0 );
      delay(1000);

      snprintf(aux_str, sizeof(aux_str), "AT+CSTT=\"%s\",\"%s\",\"%s\"", __APN, __usrnm, __password);

      // Sets the APN, user name and password
      if (sendATcommand2(aux_str, "OK",  "ERROR", 30000) == 1)
      {

        // Waits for status IP START
        if (sendATcommand("AT+CIPSTATUS", "START", 500)  == 0 )
          delay(3000);

        // Brings Up Wireless Connection
        if (sendATcommand2("AT+CIICR", "OK", "ERROR", 30000) == 1)
        {

          // Waits for status IP GPRSACT
          while (sendATcommand("AT+CIPSTATUS", "GPRSACT", 500)  == 0 );
          delay(3000);

          // Gets Local IP Address
          if (sendATcommand2("AT+CIFSR", ".", "ERROR", 10000) == 1)
          {

            // Waits for status IP STATUS
            while (sendATcommand("AT+CIPSTATUS", "IP STATUS", 500)  == 0 );
            //delay(5000);
            delay(5000);

            Serial.println(F("Opening TCP"));
            snprintf(aux_str, sizeof(aux_str), "AT+CIPSTART=\"TCP\",\"%s\",\"%s\"", MQTTHost, MQTTPort);

            // Opens a TCP socket
            if (sendATcommand2(aux_str, "OK\r\n\r\nCONNECT", "CONNECT FAIL", 30000) == 1)
            {

              // Serial.println(F("Connected"));
              return 1;

            }

            else
            {
              Serial.println(F("Error opening the connection"));
              Serial.println(F("UNABLE TO CONNECT TO SERVER "));
              return 0;

            }

          }
          else
          {
            Serial.println(F("ERROR GETTING IP ADDRESS "));
            return 0;

          }
        }
        else
        {
          Serial.println(F("ERROR BRINGING UP WIRELESS CONNECTION"));
          return 0;
        }
      }
      else {
        Serial.println(F("Error setting the APN"));
        return 0;
      }


    }
    else
    {
      Serial.println(F("Error setting CIPRXGET"));
      return 0;
    }


  }
}



