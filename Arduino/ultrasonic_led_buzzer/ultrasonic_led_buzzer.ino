int Trig = 6; int Echo = 7;
#define LED_RED 10
#define LED_BLU 9
#define LED_GRN 8
#define BUZZER  5
void setup() {
  Serial.begin(9600);
  pinMode(Trig, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLU, OUTPUT);
  pinMode(LED_GRN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(Echo, INPUT);
}
void loop() {
  digitalWrite(Trig, LOW);
  digitalWrite(Echo, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  unsigned long duration = pulseIn(Echo, HIGH);  
  float distance = ((float)(340*duration)/10000) / 2;
  Serial.print(distance);
  Serial.println("cm");
  
  if(distance > 200 && distance < 250)
      digitalWrite(LED_GRN, HIGH);
  
  if(distance > 100 && distance < 200)
  {
    digitalWrite(LED_BLU, HIGH);
    digitalWrite(BUZZER, HIGH);
  }
  
  if(distance > 1 && distance < 100)
  {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(BUZZER, HIGH);
  }
  if(distance > 250)
  {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BLU, LOW);
    digitalWrite(LED_GRN, LOW);
    digitalWrite(BUZZER, LOW);
  }
    
}
