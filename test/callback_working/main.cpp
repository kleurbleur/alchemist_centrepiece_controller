#include <Arduino.h>

void inductive_sensor_A_top(int start){
  if (start == 1)
  {
    Serial.println("start 1");
  } 
  else if (start == 0)
  {
    Serial.println("start 0");
  }  
}

// helper function to check input and start the right function
void inputCheckFunction(const int inputPin, bool input, void (*input_callback)(int))
{
  if (digitalRead(inputPin) && input == false)
    {
      input = true;
      Serial.printf("input %i: 1\n", inputPin);
      input_callback(1);
    }else if (!digitalRead(inputPin) && input == false)
    {
      Serial.printf("input %i: 0\n", inputPin);
      input_callback(0);
  }

}


void setup()
{
    Serial.begin(115200);
    pinMode(39, INPUT_PULLUP);
}

void loop()
{
    inputCheckFunction(39, false, &inductive_sensor_A_top);
}