// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#include <Arduino.h>

#define out1 2
#define out2 3
#define out3 4
#define out4 5

const int del = 5000;

void setup() {
  // Initialize Serial port
  Serial.begin(115200);
  while (!Serial) continue;

  pinMode(out1, OUTPUT);
  pinMode(out2, OUTPUT);
  pinMode(out3, OUTPUT);
  pinMode(out4, OUTPUT);

}

void loop() {
    delay(del);
    digitalWrite(out1, HIGH);
    digitalWrite(out2, HIGH);
    digitalWrite(out3, HIGH);
    digitalWrite(out4, HIGH);
    Serial.println("HIGH");
    delay(del);
    digitalWrite(out1, LOW);
    digitalWrite(out2, LOW);
    digitalWrite(out3, LOW);
    digitalWrite(out4, LOW);
    Serial.println("LOW");
}