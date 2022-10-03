// working test code for the IFS253 inductive sensor connected used with a optocoupler board and 24V psu. 
// black wire is signal on 
// brown v+
// blue v- 

#include <Arduino.h>

const int ind_s1 = 32;

void setup() {
  Serial.begin(115200);
  Serial.println("Conductive MQTT Sensor");
  pinMode(ind_s1, INPUT_PULLUP);
}

void loop() {
  int val = digitalRead(ind_s1);
  Serial.println(val);
  delay(150);
}