#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_TCA9534.h>


TCA9534 GPIO_1;
TCA9534 GPIO_2;


void setup()
{
    Serial.begin(115200);
    Serial.println("Qwiic GPIO Example 1a- Write GPIO");
    
    Wire.begin();
    if (GPIO_1.begin(Wire, 0x27) == false) {
        Serial.println("Check Connections. No Qwiic GPIO detected at 27.");
        while (1);
    }
    // if (GPIO_2.begin(Wire, 0x26) == false) {
    //     Serial.println("Check Connections. No Qwiic GPIO detected at 26.");
    //     while (1);
    // }    

}

void loop()
{
    GPIO_1.digitalWrite(0,HIGH);
    // GPIO_2.digitalWrite(6,HIGH);
    Serial.println("HIGH");
    delay(3000);
    GPIO_1.digitalWrite(0,LOW);
    // GPIO_2.digitalWrite(6,LOW);
    Serial.println("LOW");
    delay(3000);

}