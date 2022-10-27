#include <Arduino.h>
#include <Wire.h>           // Include the I2C library (required)
#include <SparkFunSX1509.h> //Click here for the library: http://librarymanager/All#SparkFun_SX1509


// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E; // SX1509 I2C address
SX1509 io;                        // Create an SX1509 object to be used throughout
// SX1509 pin definitions:
const byte PIN_1 = 0; 
const byte PIN_2 = 1; 
const byte PIN_3 = 2; 
const byte PIN_4 = 3; 

const byte PIN_5 = 4; 
const byte PIN_6 = 5; 
const byte PIN_7 = 6; 
const byte PIN_8 = 7; 

const byte PIN_9 = 8; 
const byte PIN_10 = 9; 
const byte PIN_11 = 10; 
const byte PIN_12 = 11; 



void setup()
{
  Serial.begin(115200);
  Serial.println("SX1509 Example");

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
  io.pinMode(PIN_2, OUTPUT);
  io.pinMode(PIN_3, OUTPUT);
  io.pinMode(PIN_4, OUTPUT);
  io.pinMode(PIN_5, OUTPUT);
  io.pinMode(PIN_6, OUTPUT);
  io.pinMode(PIN_7, OUTPUT);
  io.pinMode(PIN_8, OUTPUT);
  io.pinMode(PIN_9, OUTPUT);
  io.pinMode(PIN_10, OUTPUT);
  io.pinMode(PIN_11, OUTPUT);
  io.pinMode(PIN_12, OUTPUT);    

}


void loop()
{
  Serial.println("HIGH");
  io.digitalWrite(PIN_1, HIGH);
  io.digitalWrite(PIN_2, HIGH);
  io.digitalWrite(PIN_3, HIGH);
  io.digitalWrite(PIN_4, HIGH);
  io.digitalWrite(PIN_5, HIGH);
  io.digitalWrite(PIN_6, HIGH);
  io.digitalWrite(PIN_7, HIGH);
  io.digitalWrite(PIN_8, HIGH);
  io.digitalWrite(PIN_9, HIGH);
  io.digitalWrite(PIN_10, HIGH);
  io.digitalWrite(PIN_11, HIGH);
  io.digitalWrite(PIN_12, HIGH);    
  delay(5000);   
  Serial.println("LOW");                          // Delay half-a-second
  io.digitalWrite(PIN_1, LOW);
  io.digitalWrite(PIN_2, LOW); 
  io.digitalWrite(PIN_3, LOW); 
  io.digitalWrite(PIN_4, LOW); 
  io.digitalWrite(PIN_5, LOW);
  io.digitalWrite(PIN_6, LOW); 
  io.digitalWrite(PIN_7, LOW); 
  io.digitalWrite(PIN_8, LOW); 
  io.digitalWrite(PIN_9, LOW);
  io.digitalWrite(PIN_10, LOW); 
  io.digitalWrite(PIN_11, LOW); 
  io.digitalWrite(PIN_12, LOW);     
  delay(5000);                           // Delay half-a-second

}