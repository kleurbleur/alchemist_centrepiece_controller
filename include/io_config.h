#ifndef IO_CONFIG_H
#define IO_CONFIG_H

#include <Arduino.h>

// PIN ASSIGNMENT
// OUT
#define motor_controller_arm_A_start_pin 5      //SX1509
#define motor_controller_arm_A_end_pin 6       //SX1509
#define motor_controller_arm_A_pause_pin 7      //SX1509
#define motor_controller_arm_A_enable_pin 4     //SX1509
#define arm_A_solenoid_safety_pin 32            //mod-io relay board
#define motor_controller_arm_B_start_pin 9      //SX1509
#define motor_controller_arm_B_end_pin 10       //SX1509
#define motor_controller_arm_B_pause_pin 11     //SX1509
#define motor_controller_arm_B_enable_pin 8     //SX1509
#define arm_B_solenoid_safety_pin 36            //mod-io relay board
#define motor_controller_rings_start_pin 1      //SX1509
#define motor_controller_rings_enable_pin 0     //SX1509
#define motor_controller_rings_pause_pin 2      //SX1509
#define motor_controller_rings_resume_pin 3     //SX1509
#define led_hole_pin 39                         //ESP32 io output (nog te solderen)
// IN
#define inductive_sensor_A_top_pin 13        // mod-io opto board
#define inductive_sensor_A_bottom_pin 39     // mod-io opto board
#define motor_control_A_up_pin 14            // ESP32 interrupt input (nog te programmeren)
#define motor_control_A_bottom_pin 32        // ESP32 interrupt input (nog te programmeren)
#define inductive_sensor_B_top_pin = 33;        // mod-io opto board
#define inductive_sensor_B_bottom_pin = 34;     // mod-io opto board
#define motor_control_B_up_pin = 35;            // ESP32 interrupt input (nog te programmeren)
#define motor_control_B_bottom_pin = 36;        // ESP32 interrupt input (nog te programmeren)
#define motor_ring_running_pin = 3;             // ESP32 interrupt input (nog te programmeren)
#define motor_ring_targer_reached_pin = 4;      // ESP32 interrupt input (nog te programmeren)



//in- and outputs --- these needs to correspond to the document at https://docs.google.com/document/d/1GiCjMT_ph-NuIOsD4InIvT-H3MmUkSkzBZRMM1L5IsI/edit#heading=h.wqfd6v7o79qu
#define NUM_OUTPUTS 9          // amount of outputs
#define START_OUTPUT 1          // the start number of the output
#define NUM_INPUTS 10            // amount of inputs
#define START_INPUT 17          // the start number of the input


// naming the IDs and values
// these names are now numbered and can be used when calling an array numbers. Instead of inIDs[2] now inIDs[arm_A_controller_top]
enum inputs{
  TOP_SENSOR_CONTROLLER_ARM_A,
  BOTTOM_SENSOR_CONTROLLER_ARM_A,
  TOP_CONTROLLER_ARM_A,
  MOVING_CONTROLLER_ARM_A,
  BOTTOM_CONTROLLER_ARM_A,
  TOP_SENSOR_CONTROLLER_ARM_B,
  BOTTOM_SENSOR_CONTROLLER_ARM_B,
  TOP_CONTROLLER_ARM_B,
  MOVING_CONTROLLER_ARM_B,
  BOTTOM_CONTROLLER_ARM_B,
  MOVING_CONTROLLER_RINGS,
  TARGET_REACHED_CONTROLLER_RINGS
};
enum outputs{
  TOP_CONTROLLER_ARMS,
  BOTTOM_CONTROLLER_ARMS,
  ENABLE_CONTROLLER_ARMS,
  PAUSE_CONTROLLER_ARMS,
  SOLENOID_CONTROLLER_ARMS,
  START_CONTROLLER_RINGS,
  ENABLE_CONTROLLER_RINGS,
  PAUSE_CONTROLLER_RINGS,
  RESUME_CONTROLLER_RINGS,
  LED_HOLE
};



// and here we create the id and the value arrays which are callable by e.g. outIDs[led_hole] (which is 15)
int outIDs[NUM_OUTPUTS];
int outValues[NUM_OUTPUTS] = {0};
int inIDs[NUM_INPUTS];
int inValues[NUM_INPUTS] = {0};



// and two helper functions to set the id's of the out- and inputs.
void setOutputsNum()
{
  Serial.print("Outputs[value]: {");
  for(int i = 0; i < NUM_OUTPUTS; i++)
  {
    outIDs[i] = i+START_OUTPUT;
    Serial.print(i+START_OUTPUT);
    Serial.printf("[%i]", outValues[i]);    
    if (i < NUM_OUTPUTS-1)
    {    
      Serial.print(", ");
    }
    if (i >= NUM_OUTPUTS-1)
    {
      Serial.println("}");
    }
  }
}
void setInputsNum()
{
  Serial.print("Inputs: {");
  for(int i = 0; i < NUM_INPUTS; i++)
  {
    inIDs[i] = i+START_INPUT;
    Serial.print(i+START_INPUT);
    Serial.printf("[%i]", inValues[i]);
    if (i < NUM_INPUTS-1)
    {
      Serial.print(", ");
    }
    if (i >= NUM_INPUTS-1)
    {
      Serial.println("}");
    }    
  }
}

#endif    // IO_CONFIG_H