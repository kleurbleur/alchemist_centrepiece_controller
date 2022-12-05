/*
Nanotec N5-2 controller sync test
*/

#include <Arduino.h>
#include <Wire.h>
#include <SparkFunSX1509.h>


const u_int8_t del_time = 25;


// pin assignment
// OUT
#define motor_controller_arm_A_bottom_pin 2     //SX1509
#define motor_controller_arm_A_top_pin 1        //SX1509
#define motor_controller_arm_A_pause_pin 3      //SX1509
#define motor_controller_arm_A_enable_pin 0     //SX1509
#define arm_A_solenoid_safety_pin RELAY1        //mod-io board relay 
#define motor_controller_arm_B_bottom_pin 6     //SX1509
#define motor_controller_arm_B_top_pin 5        //SX1509
#define motor_controller_arm_B_pause_pin 7      //SX1509
#define motor_controller_arm_B_enable_pin 4     //SX1509
#define arm_B_solenoid_safety_pin RELAY2        //mod-io board relay 
#define motor_controller_rings_start_pin 9      //SX1509
#define motor_controller_rings_enable_pin 8     //SX1509
#define motor_controller_rings_pause_pin 10     //SX1509
#define motor_controller_rings_resume_pin 11    //SX1509
#define led_hole_pin 32                         //ESP32 io output



const byte SX1509_ADDRESS = 0x3E;     // io Externder SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default 0x3E):
SX1509 io;    

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
bool arms_enable = false;
bool arms_top = false;
bool arms_bottom = false;
bool arms_pause = false;
bool rings_enable = false;
bool rings_start = false;
bool rings_pause = false;
bool rings_resume = false;
bool enable = false;

void status()
{
    Serial.printf(" %i%i%i%i %i%i%i\n", arms_enable,arms_top,arms_bottom,arms_pause,rings_enable,rings_start,rings_pause);
}


void setup() {
  // initialize serial:
  Serial.begin(115200);

  delay(1500);

  Wire.begin(5, 15);
  if (io.begin(SX1509_ADDRESS) == false)
  {
    Serial.println("Failed to communicate. Check wiring and address of SX1509.");
  }
  else if (io.begin(SX1509_ADDRESS) == true)
  {
    Serial.println("SX1509 addressed");
    io.pinMode(motor_controller_arm_A_enable_pin, OUTPUT);  //Use io_OUT and io_IN instead of OUTPUT and INPUT_PULLUP
    io.pinMode(motor_controller_arm_A_top_pin, OUTPUT);
    io.pinMode(motor_controller_arm_A_bottom_pin, OUTPUT);
    io.pinMode(motor_controller_arm_A_pause_pin, OUTPUT);
    io.pinMode(motor_controller_arm_B_enable_pin, OUTPUT);  //Use io_OUT and io_IN instead of OUTPUT and INPUT_PULLUP
    io.pinMode(motor_controller_arm_B_top_pin, OUTPUT);
    io.pinMode(motor_controller_arm_B_bottom_pin, OUTPUT);
    io.pinMode(motor_controller_arm_B_pause_pin, OUTPUT);
    io.pinMode(motor_controller_rings_enable_pin, OUTPUT);  //Use io_OUT and io_IN instead of OUTPUT and INPUT_PULLUP
    io.pinMode(motor_controller_rings_start_pin, OUTPUT);
    io.pinMode(motor_controller_rings_pause_pin, OUTPUT);
    io.pinMode(motor_controller_rings_resume_pin, OUTPUT);
  }
  
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);

  Serial.println("Disabled. Press e to enable and use 1,2,3,4 and 7,8,9 to control the arms and rings.");
}

void loop() {
  // print the string when a newline arrives:
  if (stringComplete) {
//    Serial.print(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }

    if (inChar == 'e')
    {
        enable = !enable;
    }

    if (enable)
    {
        // check for input and set the outputs accordingly
        if (inChar == '1'){
            Serial.print("Arms Enable: ");
            arms_enable = !arms_enable; 
            io.digitalWrite(motor_controller_arm_A_enable_pin, arms_enable);
            io.digitalWrite(motor_controller_arm_B_enable_pin, arms_enable);
            status();
        }
        else if (inChar == '2'){
        Serial.print("Arms Top: ");
            arms_top = !arms_top; 
            io.digitalWrite(motor_controller_arm_A_top_pin, arms_top);
            io.digitalWrite(motor_controller_arm_B_top_pin, arms_top);
            status();
            delay(del_time);
            arms_top = !arms_top;
            io.digitalWrite(motor_controller_arm_A_top_pin, arms_top);
            io.digitalWrite(motor_controller_arm_B_top_pin, arms_top);
            status();
        }
        else if (inChar == '3'){
        Serial.print("Arms Bottom: ");
            arms_bottom = !arms_bottom; 
            io.digitalWrite(motor_controller_arm_A_bottom_pin, arms_bottom);
            io.digitalWrite(motor_controller_arm_B_bottom_pin, arms_bottom);
            status();
            delay(del_time);
            arms_bottom = !arms_bottom; 
            io.digitalWrite(motor_controller_arm_A_bottom_pin, arms_bottom);
            io.digitalWrite(motor_controller_arm_B_bottom_pin, arms_bottom);
            status();
        }   
        else if (inChar == '4'){
        Serial.print("Arms Pause: ");
            arms_pause = !arms_pause; 
            io.digitalWrite(motor_controller_arm_A_pause_pin, arms_pause);
            io.digitalWrite(motor_controller_arm_B_pause_pin, arms_pause);
            status();
            delay(del_time);
            arms_pause = !arms_pause; 
            io.digitalWrite(motor_controller_arm_A_pause_pin, arms_pause);
            io.digitalWrite(motor_controller_arm_B_pause_pin, arms_pause);
            status();
        }
        else if (inChar == '7'){
        Serial.print("Rings Enable");
            rings_enable = !rings_enable; 
            io.digitalWrite(motor_controller_rings_enable_pin, rings_enable);
            status();            
        }
        else if (inChar == '8'){
        Serial.print("Rings Start: ");
            rings_start = !rings_start; 
            io.digitalWrite(motor_controller_rings_start_pin, rings_start);
            status();
            delay(del_time);
            rings_start = !rings_start; 
            io.digitalWrite(motor_controller_rings_start_pin, rings_start);
            status();            
        }
        else if (inChar == '9'){
        Serial.print("Rings Pause: ");
            rings_pause = !rings_pause; 
            io.digitalWrite(motor_controller_rings_pause_pin, rings_pause);
            status();
            delay(del_time);
            rings_pause = !rings_pause; 
            io.digitalWrite(motor_controller_rings_pause_pin, rings_pause);
            status();
        }      
    } 

  }
}
