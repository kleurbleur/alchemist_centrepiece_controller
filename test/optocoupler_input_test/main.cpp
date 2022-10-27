#include <Arduino.h>

#define opto_1 14
#define opto_2 33
#define opto_3 36
#define opto_4 39
#define opto_5 3
#define opto_6 4

void setup()
{
    
Serial.begin(115200);
pinMode(opto_1, INPUT_PULLUP);
pinMode(opto_2, INPUT_PULLUP);
pinMode(opto_3, INPUT_PULLUP);
pinMode(opto_4, INPUT_PULLUP);
pinMode(opto_5, INPUT_PULLUP);
pinMode(opto_6, INPUT_PULLUP);


}


void loop()
{

// int val = digitalRead(opto_1);
Serial.print("optos: ");
Serial.print(digitalRead(opto_1));
Serial.print(digitalRead(opto_2));
Serial.print(digitalRead(opto_3));
Serial.print(digitalRead(opto_4));
Serial.print(digitalRead(opto_5));
Serial.println(digitalRead(opto_6));

delay(100);

}