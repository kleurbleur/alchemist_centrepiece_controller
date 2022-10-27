#include <Arduino.h>
#include <SparkFunSX1509.h> //Click here for the library: http://librarymanager/All#SparkFun_SX1509

// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E; // SX1509 I2C address
SX1509 io;                        // Create an SX1509 object to be used throughout
// SX1509 pin definitions:
const byte PIN_1 = 0; 


struct Button {
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
};

Button button1 = {39, 0, false};

//variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;  
unsigned long last_button_time = 0; 

void IRAM_ATTR isr() {
    button_time = millis();
if (button_time - last_button_time > 10)
{
        button1.numberKeyPresses++;
        button1.pressed = true;
        last_button_time = button_time;
}
}

void setup() {
    Serial.begin(115200);
    pinMode(button1.PIN, INPUT);
    attachInterrupt(button1.PIN, isr, ONLOW); // HIGH, ONLOW, RISING, CHANGE, FALLING
}

void loop() {
    if (button1.pressed) {
        Serial.printf("Button has been pressed %u times\n", button1.numberKeyPresses);
        button1.pressed = false;
    }

}