//===================
// Using I2C to send and receive structs between two Arduinos
//   SDA is the data connection and SCL is the clock connection
//   On an Uno  SDA is A4 and SCL is A5
//   On an Mega SDA is 20 and SCL is 21
//   GNDs must also be connected
//===================


#include <Arduino.h>
#include <Wire.h>


struct motorStruct {
    uint8_t arm_A_start;                 
    uint8_t arm_A_end;                   
    uint8_t arm_A_pause;                 
    uint8_t arm_A_enable;                
    uint8_t arm_B_start;                 
    uint8_t arm_B_end;                   
    uint8_t arm_B_pause;                 
    uint8_t arm_B_enable;                
    uint8_t arm_rings_start;             
    uint8_t arm_rings_enable;           
    uint8_t arm_rings_pause;            
    uint8_t arm_rings_continue;          
};

motorStruct motor = {0};

bool newTxData = false;
bool test_if = false;


        // I2C control stuff

const byte thisAddress = 8; // these need to be swapped for the other Arduino
const byte otherAddress = 9;


        // timing variables
unsigned long prevUpdateTime = 0;
unsigned long updateInterval = 500;



void updateDataToSend() {

    if (millis() - prevUpdateTime >= updateInterval) {
        prevUpdateTime = millis();
        if (newTxData == false) { // ensure previous message has been sent
            if (!test_if)
            {
              motor.arm_A_enable = 1;  
              motor.arm_B_enable = 1;            
              test_if = true;
            }
            else if (test_if)
            {
              motor.arm_A_enable = 0;  
              motor.arm_B_enable = 0;            
              test_if = false;
            }          
            newTxData = true;
        }
    }
}

//============

void transmitData() {

    if (newTxData == true) {
        Wire.beginTransmission(otherAddress);
        Wire.write((byte*) &motor, sizeof(motor));
        Wire.endTransmission();    // this is what actually sends the data

        // for demo show the data that as been sent
        Serial.print("Sent ");
        Serial.print(motor.arm_A_enable);
        Serial.print(' ');
        Serial.print(motor.arm_B_enable);

        newTxData = false;
    }
}

//=================================

void setup() {
    Serial.begin(115200);
    Serial.println("\nStarting I2C Master demo\n");

        // set up I2C
    Wire.begin(16,13); // join i2c bus

}

//============

void loop() {

        // this function updates the data in txData
    updateDataToSend();
        // this function sends the data if one is ready to be sent
    transmitData();
}

//============
