#include <Arduino.h>



int dutyCycle = 0;
int led_start = 1;

int previousMicros= 0;
int interval = 500;


void setup()
{
  Serial.begin(115200);
  delay(5000);

}

void loop()
{

unsigned long currentMicros = micros();


// increase the LED brightness
  if ( led_start == 1){
    if (currentMicros - previousMicros >= interval) {
      previousMicros = currentMicros;   
      dutyCycle++;
      Serial.println(dutyCycle);
      if (dutyCycle == 4096){
        led_start = 0;
        Serial.println("led_hole fade in done");
      }
    } 
  }

}
