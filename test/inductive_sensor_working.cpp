// working test code for the IFS253 inductive sensor connected used with a optocoupler board and 24V psu. 
// black wire is signal on 
// brown v+
// blue v- 

#include <Arduino.h>

const int ind_s1 = 13;
const int ind_s2 = 14;


void setup() {
  Serial.begin(115200);
  Serial.println("Conductive MQTT Sensor");
  pinMode(ind_s1, INPUT_PULLUP);
  pinMode(ind_s2, INPUT_PULLUP);
}

void loop() {
  int val1 = digitalRead(ind_s1);
  int val2 = digitalRead(ind_s2);
  Serial.print("S1: ");
  Serial.print(val1);
  Serial.print(" - S2: ");
  Serial.println(val2);
  delay(150);
}