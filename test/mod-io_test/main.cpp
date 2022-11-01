//The example turns on and off all relays of MOD-IO.
//It also reads and prints all digital inputs and all 
//analog inputs over the serial monitor
#include <Arduino.h>
#include <Wire.h>
#include <MOD_IO.h>

// Control MOD-IO at address 0x2C (the 7-bit address), 
// shifted 1-bit left the address is 0x58.
// Be sure to enter the correct I2C address of your module

#define	PERIOD	1000	// ms

MOD_IO modio(0x58);

void setup () {
  Serial.begin(115200);
  modio.begin();
}

void loop () {

//Definition of variables

  uint8_t i;
  uint8_t val;
  uint8_t val4;
  uint8_t val3;
  uint8_t val2;
  uint8_t val1;
  uint16_t adc;
  float voltage = 0;

//Relays turning on and off. Note that the relays require external power supply
//provided to the PWR_J of the MOD-IO board

  modio.setRelay(RELAY1, 1);
  delay(PERIOD);
  modio.setRelay(RELAY1, 0);
  
  modio.setRelay(RELAY2, 1);
  delay(PERIOD);
  modio.setRelay(RELAY2, 0);
  
  modio.setRelay(RELAY3, 1);
  delay(PERIOD);
  modio.setRelay(RELAY3, 0);
  
  modio.setRelay(RELAY4, 1);
  delay(PERIOD);
  modio.setRelay(RELAY4, 0);
  
  delay(2000);

//Reads and prints the digital inputs 

  val = modio.digitalReadAll();
  Serial.print("digital Inputs: ");
  Serial.println(val);
  val4 = modio.digitalRead(4);
  Serial.print("digital Input 4: ");
  Serial.println(val4);
  val3 = modio.digitalRead(3);
  Serial.print("digital Input 3: ");
  Serial.println(val3);
  val2 = modio.digitalRead(2);
  Serial.print("digital Input 2: ");
  Serial.println(val2);
  val1 = modio.digitalRead(1);
  Serial.print("digital Input 1: ");
  Serial.println(val1);  

//Reads and prints the analog inputs and their value 

  // Serial.print("analog Inputs: ");
  // for (i=0; i<4; i++)
  // {
  //   adc = modio.analogRead(i);
  //   voltage = (adc/1023.0)*3.3;
  //   Serial.print(voltage, 2);
  //   Serial.println("V");
  // }
    
  delay(500);
}
