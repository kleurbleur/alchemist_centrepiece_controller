#include <Arduino.h>
#include <Wire.h>           // Include the I2C library (required)
#include <SparkFunSX1509.h> //Click here for the library: http://librarymanager/All#SparkFun_SX1509


// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E; // SX1509 I2C address
SX1509 io;                        // Create an SX1509 object to be used throughout
// SX1509 pin definitions:
const byte PIN_1 = 0; 
const byte PIN_2 = 1; 
const byte PIN_3 = 6; 
const byte PIN_4 = 7; 



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

}


void loop()
{
  Serial.println("HIGH");
  io.digitalWrite(PIN_1, HIGH);
  io.digitalWrite(PIN_2, HIGH);
  io.digitalWrite(PIN_3, HIGH);
  io.digitalWrite(PIN_4, HIGH);
  delay(5000);   
  Serial.println("LOW");                          // Delay half-a-second
  io.digitalWrite(PIN_1, LOW);
  io.digitalWrite(PIN_2, LOW); 
  io.digitalWrite(PIN_3, LOW); 
  io.digitalWrite(PIN_4, LOW); 
  delay(5000);                           // Delay half-a-second

}