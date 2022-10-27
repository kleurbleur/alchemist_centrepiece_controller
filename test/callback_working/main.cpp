#include <Arduino.h>
#include <Wire.h>           // Include the I2C library (required)
#include <SparkFunSX1509.h> //Click here for the library: http://librarymanager/All#SparkFun_SX1509


// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E; // SX1509 I2C address
SX1509 io;                        // Create an SX1509 object to be used throughout
// SX1509 pin definitions:
const byte PIN_1 = 0; 

// inID's and inValues moeten gezet worden op basis van de digitalRead. Wanneer dat gedaan is dan kan je ook dat ook in de if statement checken. 

const int inductive_sensor_A_top_pin = 39;

int inIDS[3] = {1,2,3};
int inValues[3] = {0};

unsigned long button_time = 0;  
unsigned long last_button_time = 0; 
unsigned long high_time = 0;  
unsigned long last_high_time = 0; 
unsigned long low_time = 0;  
unsigned long last_low_time = 0; 
bool relay_state = false;

void inductive_sensor_A_top(int start){
  if (start == 1)
  {
    Serial.print("start = 1: ");
    Serial.println(inValues[0]);
  } 
  else if (start == 0)
  {
    Serial.print("start = 0: ");
    Serial.println(inValues[0]);
  }  
}

// helper function to check input and start the right function
void inputCheckFunction(const int inputPin, const int id, void (*input_callback)(int))
{
    int valuePOS =  id - 1;
    button_time = millis();
    if (button_time - last_button_time > 10)
    {
        if (digitalRead(inputPin) && inValues[valuePOS] == 0) // dit moet gelinkt worden aan de inputState
        {
            inValues[valuePOS] = 1;
            // Serial.printf("input %i: 1\n", inputPin);
            input_callback(1);
        }
        else if (!digitalRead(inputPin) && inValues[valuePOS] == 1)
        {
            inValues[valuePOS] = 0;
            // Serial.printf("input %i: 0\n", inputPin);
            input_callback(0);
        }
        last_button_time = button_time;  
    }
}


void setup()
{
    Serial.begin(115200);
    pinMode(inductive_sensor_A_top_pin, INPUT); // 36 t/m 39 heeft geen pullup -> dubbel testen

  Wire.begin(5,13);

  // Call io.begin(<address>) to initialize the SX1509. If it
  // successfully communicates, it'll return 1.
  if (io.begin(SX1509_ADDRESS) == false)
  {
    Serial.println("Failed to communicate. Check wiring and address of SX1509.");
    while (1)
      ; // If we fail to communicate, loop forever.
  }

  // Call io.pinMode(<pin>, <mode>) to set an SX1509 pin as
  // an output:
  io.pinMode(PIN_1, OUTPUT);


}

void loop()
{
    inputCheckFunction(inductive_sensor_A_top_pin,1, &inductive_sensor_A_top);


    high_time = millis();
    if (high_time - last_high_time > 5000)
    {
      relay_state = !relay_state;
      if (relay_state)
      {
        Serial.println("HIGH");
        io.digitalWrite(PIN_1, HIGH);
      }
      else if (!relay_state)
      {
        Serial.println("LOW");
        io.digitalWrite(PIN_1, LOW);
      }   
      last_high_time = high_time;
    }



}