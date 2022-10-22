#include <Arduino.h>

#define opto_1 2


void setup()
{
    
Serial.begin(115200);
pinMode(opto_1, INPUT_PULLUP);

}


void loop()
{

int val = digitalRead(opto_1);
Serial.println(val);
delay(200);

}