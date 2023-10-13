#include <Arduino.h>

// put function declarations here:
int dataPin = 5;

void setup() {
  // put your setup code here, to run once:
  pinMode(dataPin, OUTPUT);
  Serial.begin(9600);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(dataPin,127);//Simple Square Wave at 50% duty cycle
}

