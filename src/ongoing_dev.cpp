#include <Arduino.h>
#include <SPI.h>
#include <ETH.h>
#include <PubSubClient.h>
#include <Sherlocked.h>
#include <Wire.h>
#include <SparkFunSX1509.h>
#include <MOD_IO.h>

// Firmware version
char firmware_version[] = "0.8";                                // final pin lay out

// SETTINGS

// mqtt/ace settings
char hostname[] ="centerpiece-control";                                  // the hostname for board  <= REPLACE
const char gen_topic[] = "alch";  
const char puzzle_topic[] = "alch/centerpiece"; 
const char module_topic[] = "alch/centerpiece/centerpiece-control";      // the module name of the board <= REPLACE
IPAddress server(10, 0, 0, 10);                                 // ip address of the mqtt/ace server <= REPLACE


// controller settings
//pwm 
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 12;

// motor controllers
const int motor_controller_on_off_delay = 2500;  // delay between the HIGH and LOW for the motor controllers in microseconds

// input debounce time
const int debounce_time = 10;


// INPUTS and OUTPUTS
// These needs to correspond to the document at https://docs.google.com/document/d/1GiCjMT_ph-NuIOsD4InIvT-H3MmUkSkzBZRMM1L5IsI/edit#heading=h.wqfd6v7o79qu

// how many do we have and do they need to start at a certain id? 
#define NUM_OUTPUTS 9           // amount of outputs
#define START_OUTPUT 1          // the start number of the output
#define NUM_INPUTS 10           // amount of inputs
#define START_INPUT 17          // the start number of the input

// pin assignment
// IN
#define motor_control_A_top_pin 14              // ESP32 interrupt input (nog te programmeren)
#define motor_control_A_bottom_pin 33           // ESP32 interrupt input (nog te programmeren)
#define motor_control_B_top_pin 36              // ESP32 interrupt input (nog te programmeren)
#define motor_control_B_bottom_pin 39           // ESP32 interrupt input (nog te programmeren)
#define motor_ring_running_pin 3                // ESP32 interrupt input (nog te programmeren)
#define motor_ring_target_reached_pin 4         // ESP32 interrupt input (nog te programmeren)
#define inductive_sensor_A_top_pin 1            // mod-io board opto 
#define inductive_sensor_A_bottom_pin 2         // mod-io board opto
#define inductive_sensor_B_top_pin 3            // mod-io board opto
#define inductive_sensor_B_bottom_pin 4         // mod-io board opto
// OUT
// motor controller arm a pins
#define motor_controller_arm_A_enable_pin 0     //SX1509
#define motor_controller_arm_A_top_pin 1        //SX1509
#define motor_controller_arm_A_bottom_pin 2     //SX1509
#define motor_controller_arm_A_pause_pin 3      //SX1509
#define arm_A_solenoid_safety_pin RELAY1        //mod-io board relay 
// motor controller arm b pins
#define motor_controller_arm_B_enable_pin 8     //SX1509
#define motor_controller_arm_B_top_pin 9        //SX1509
#define motor_controller_arm_B_bottom_pin 10    //SX1509
#define motor_controller_arm_B_pause_pin 11     //SX1509
#define arm_B_solenoid_safety_pin RELAY2        //mod-io board relay 
// rings pins
#define motor_controller_rings_enable_pin 4     //SX1509
#define motor_controller_rings_start_pin 5      //SX1509
#define motor_controller_rings_pause_pin 6      //SX1509
#define motor_controller_rings_resume_pin 7     //SX1509
#define led_hole_pin 32                         //ESP32 io output


// lets put these pins in an array
int _inputsPins[6] = {
  motor_control_A_top_pin,
  motor_control_A_bottom_pin,
  motor_control_B_top_pin,
  motor_control_B_bottom_pin,
  motor_ring_running_pin,
  motor_ring_target_reached_pin,
};
// lets calculate the size of this array so we can loop through it later on
int _inputsPinsArraySize = sizeof(_inputsPins)/sizeof(int);

// naming the IDs and values
// these names are now numbered and can be used when calling an array numbers. Instead of inIDs[2] now inIDs[arm_A_controller_top]
enum inputs{
  TOP_CONTROLLER_ARM_A,
  BOTTOM_CONTROLLER_ARM_A,
  TOP_CONTROLLER_ARM_B,
  BOTTOM_CONTROLLER_ARM_B,
  MOVING_CONTROLLER_RINGS,
  TARGET_REACHED_CONTROLLER_RINGS,
  TOP_SENSOR_CONTROLLER_ARM_A,
  BOTTOM_SENSOR_CONTROLLER_ARM_A,
  TOP_SENSOR_CONTROLLER_ARM_B,
  BOTTOM_SENSOR_CONTROLLER_ARM_B,
  MOVING_CONTROLLER_ARM_B,
  MOVING_CONTROLLER_ARM_A
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

// a struct to create more overview for the inputs
struct input {
    uint8_t id;
    uint8_t value;
    uint8_t pin;
    uint32_t current_debounce;
    uint32_t last;
    uint32_t debounce;
};

// lets create those struct based on the amount of inputs we have
input _input[NUM_INPUTS];

// let's initiate those inputs and give them id's and connect them to the physival pins
void initInputs()
{
    Serial.println("Setup commencing");
    for (int i = 0; i < NUM_INPUTS; i++)
    {
        Serial.printf("Setup %i \n", i);
        _input[i].id = i+START_INPUT;
        if (i < _inputsPinsArraySize)
        {
            _input[i].debounce = 10;
            _input[i].last = millis();
            _input[i].pin = _inputsPins[i];
            Serial.printf("input pin found! %i \n", _input[i].pin);
            pinMode(_input[i].pin, INPUT_PULLUP);
        } 
    }
}


// and here we create the id and the value arrays which are callable by e.g. outIDs[led_hole] (which is 15)
int outIDs[NUM_OUTPUTS];
int outValues[NUM_OUTPUTS] = {0};
// int inIDs[NUM_INPUTS];
// int inValues[NUM_INPUTS] = {0};
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
// void setInputsNum()
// {
//   Serial.print("Inputs: {");
//   for(int i = 0; i < NUM_INPUTS; i++)
//   {
//     inIDs[i] = i+START_INPUT;
//     Serial.print(i+START_INPUT);
//     Serial.printf("[%i]", inValues[i]);
//     if (i < NUM_INPUTS-1)
//     {
//       Serial.print(", ");
//     }
//     if (i >= NUM_INPUTS-1)
//     {
//       Serial.println("}");
//     }    
//   }
// }

// INPUT OUTPUT END ---
//  that's it, now we can call for example _input[CONTROLLER_ARM_A_TARGET_TOP].pin or _input[CONTROLLER_ARM_A_TARGET_TOP].value


// SETUP LIBS
WiFiClient ethclient;
PubSubClient client(ethclient);
const byte SX1509_ADDRESS = 0x3E;     // io Externder SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default 0x3E):
SX1509 io;    
MOD_IO modio(0x58);                    


// variables 
static bool eth_connected = false;
unsigned long previousMicros = 0;
int long led_hole_fade_interval = 500;
bool led_hole_begin = false; 
bool led_hole_end = false; 
bool rings_move = false;
bool arm_A_up = false;
bool arm_A_down = false;
bool arm_B_up = false;
bool arm_B_down= false;
bool arm_B_moving = false;
int dutyCycle = 0; 
unsigned long currentMicros;
unsigned long currentDebounceMillis;
unsigned long last_debounce_time = 0;

// the publish function for ACE/mqtt
void pubMsg(char msg[])
{
  Serial.print("pubMsg: ");
  Serial.println(msg);
  client.publish(puzzle_topic, msg);
}
// declare function prototypes 
void pubMsg_kb(const char * method, const char *param1=(char*)'\0', const char *val1=(char*)'\0', const char *param2=(char*)'\0', const char *val2=(char*)'\0' );
// actual function
void pubMsg_kb(const char * method, const char *param1, const char *val1, const char *param2, const char *val2)
{
  char jsonMsg[200], arg1[200], arg2[200];
  if (param1 && val1){
    if (val1[0]=='{' || val1[0]=='['){
      sprintf(arg1, ",\"%s\":%s", param1, val1); 
    } else {
      sprintf(arg1, ",\"%s\":\"%s\"", param1, val1); 
    }
  }
  else if (param1 && !val1){
    Serial.println("function pubMsg: Please supply a value with parameter 1");
  }
  else if (strcmp(param1,nullptr) && strcmp(val1,nullptr) ){
    memset(arg1, 0, 50);
  }  

  if (param2 && val2){
    if (val2[0]=='{' || val2[0]=='['){
      sprintf(arg2, ",\"%s\":%s", param2, val2); 
    } else {
      sprintf(arg2, ",\"%s\":\"%s\"", param2, val2); 
    }  }
  else if (param2 && !val2){
    Serial.println("function pubMsg: Please supply a value with parameter 2");
  }
  else if (strcmp(param2,nullptr) && strcmp(val2,nullptr) ){
    memset(arg2, 0, 50);
  }  

  sprintf(jsonMsg, "{\"sender\":\"%s\",\"method\":\"%s\"%s%s}", hostname, method, arg1, arg2);
  Serial.println(jsonMsg);
  client.publish(puzzle_topic, jsonMsg);
}

void setOutput(int outID, int value)
{
    outID = outID +1;
    outValues[outID] = value;
};
void setOutputWithMessage(int outID, int value, const char* request)
{
    outValues[outID] = value;
    DynamicJsonBuffer  jsonBuffer(200);
    JsonObject& root = jsonBuffer.createObject();
    root["sender"] = hostname;
    root["method"] = "info";
    JsonArray& outputs = root.createNestedArray("outputs");
    JsonObject& outid_val = outputs.createNestedObject();
    outid_val["id"] = outIDs[outID];
    outid_val["value"] = outValues[outID];
    root["trigger"] = request;
    char full_char[250];
    root.prettyPrintTo(full_char, sizeof(full_char));
    pubMsg(full_char);      
}
void blockMessage(uint8_t requested_id, uint8_t blocking_id)
{
    DynamicJsonBuffer  jsonBuffer(200);
    JsonObject& root = jsonBuffer.createObject();
    root["sender"] = hostname;
    root["method"] = "info";
    JsonArray& outputs = root.createNestedArray("outputs");
    JsonObject& outid_val = outputs.createNestedObject();
    outid_val["id"] = outIDs[requested_id];
    outid_val["value"] = outValues[requested_id];
    JsonArray& inputs = root.createNestedArray("inputs");
    JsonObject& inputsid_val = inputs.createNestedObject();
    inputsid_val["id"] = _input[blocking_id].id;
    inputsid_val["value"] = _input[blocking_id].value;
    root["trigger"] = "block";
    char full_char[250];
    root.prettyPrintTo(full_char, sizeof(full_char));
    pubMsg(full_char);  
};

// OUTPUT FUNCTIONS
// arms motor functions
void motor_controller_arms_enable(int start){
  if (start == 1)
  {
    io.digitalWrite(motor_controller_arm_A_enable_pin, HIGH);
    io.digitalWrite(motor_controller_arm_B_enable_pin, HIGH);
    outValues[ENABLE_CONTROLLER_ARMS] = 1;
    Serial.printf("ENABLE_CONTROLLER_ARMS function start 1: %i\n", outValues[ENABLE_CONTROLLER_ARMS]);
  } 
  else if (start == 0)
  {
    io.digitalWrite(motor_controller_arm_A_enable_pin, LOW);
    io.digitalWrite(motor_controller_arm_B_enable_pin, LOW);
    outValues[ENABLE_CONTROLLER_ARMS] = 0;
    Serial.printf("ENABLE_CONTROLLER_ARMS function start 0: %i\n", outValues[ENABLE_CONTROLLER_ARMS]);
  }  
}
void motor_controller_arms_top_position(int start){
  if (start == 1 && outValues[START_CONTROLLER_RINGS] == 0 && outValues[SOLENOID_CONTROLLER_ARMS] == 0)
  {
    io.digitalWrite(motor_controller_arm_A_top_pin, HIGH);
    io.digitalWrite(motor_controller_arm_B_top_pin, HIGH);
    setOutputWithMessage(TOP_CONTROLLER_ARMS, 1, "timer");
    if (currentMicros - previousMicros >= motor_controller_on_off_delay)
    {
      previousMicros = currentMicros;
      io.digitalWrite(motor_controller_arm_A_top_pin, LOW);
      io.digitalWrite(motor_controller_arm_B_top_pin, LOW);
    }     
    outValues[TOP_CONTROLLER_ARMS] = 1;
    Serial.printf("TOP_CONTROLLER_ARMS function start 1: %i\n", outValues[TOP_CONTROLLER_ARMS]);
  } 
  else if (start == 1 && START_CONTROLLER_RINGS == 1)
  {
    blockMessage(TOP_CONTROLLER_ARMS, START_CONTROLLER_RINGS);
  } 
  else if (start == 1 && SOLENOID_CONTROLLER_ARMS == 1)
  {
    blockMessage(TOP_CONTROLLER_ARMS, SOLENOID_CONTROLLER_ARMS);
  } 
  else if (start == 1 && MOVING_CONTROLLER_RINGS == true)
  {
    blockMessage(TOP_CONTROLLER_ARMS, MOVING_CONTROLLER_RINGS); 
  }    
  if (start == 0)
  {
    io.digitalWrite(motor_controller_arm_A_bottom_pin, LOW);
    io.digitalWrite(motor_controller_arm_B_bottom_pin, LOW);
    outValues[TOP_CONTROLLER_ARMS] = 0;
    Serial.printf("TOP_CONTROLLER_ARMS function start 0: %i\n", outValues[TOP_CONTROLLER_ARMS]);    
  }
}
void motor_controller_arms_bottom_position(int start){
  if (start == 1 && outValues[START_CONTROLLER_RINGS] == 0 && outValues[SOLENOID_CONTROLLER_ARMS] == 0)
  {
    io.digitalWrite(motor_controller_arm_A_bottom_pin, HIGH);
    io.digitalWrite(motor_controller_arm_B_bottom_pin, HIGH);
    setOutputWithMessage(BOTTOM_CONTROLLER_ARMS, 1, "timer");
    if (currentMicros - previousMicros >= motor_controller_on_off_delay)
    {
      previousMicros = currentMicros;
      io.digitalWrite(motor_controller_arm_A_bottom_pin, LOW);
      io.digitalWrite(motor_controller_arm_B_bottom_pin, LOW);
    }     
    outValues[BOTTOM_CONTROLLER_ARMS] = 1;
  } 
  else if (start == 1 && outValues[START_CONTROLLER_RINGS] == 1)
  {
      DynamicJsonBuffer  jsonBuffer(200);
      JsonObject& root = jsonBuffer.createObject();
      root["sender"] = hostname;
      root["method"] = "info";
      JsonArray& outputs = root.createNestedArray("outputs");
      JsonObject& outid_val = outputs.createNestedObject();
      outid_val["id"] = outIDs[1];
      outid_val["value"] = outValues[1];
      root["trigger"] = "rings still move";
      char full_char[250];
      root.prettyPrintTo(full_char, sizeof(full_char));
      pubMsg(full_char);  
  }   
  else if (start == 1 && outValues[SOLENOID_CONTROLLER_ARMS] == 1)
  {
      DynamicJsonBuffer  jsonBuffer(200);
      JsonObject& root = jsonBuffer.createObject();
      root["sender"] = hostname;
      root["method"] = "info";
      JsonArray& outputs = root.createNestedArray("outputs");
      JsonObject& outid_val = outputs.createNestedObject();
      outid_val["id"] = outIDs[1];
      outid_val["value"] = outValues[1];
      root["trigger"] = "Solenoids still extended";
      char full_char[250];
      root.prettyPrintTo(full_char, sizeof(full_char));
      pubMsg(full_char);  
  }     
  else if (start == 0)
  {
    io.digitalWrite(motor_controller_arm_A_bottom_pin, LOW);
    io.digitalWrite(motor_controller_arm_B_bottom_pin, LOW);
    outValues[BOTTOM_CONTROLLER_ARMS] = 0;
  }
}
void motor_controller_arms_pause(int start){
  if (start == 1)
  {
    io.digitalWrite(motor_controller_arm_A_pause_pin, HIGH);
    io.digitalWrite(motor_controller_arm_B_pause_pin, HIGH);
    setOutputWithMessage(PAUSE_CONTROLLER_ARMS, 1, "timer");
    if (currentMicros - previousMicros >= motor_controller_on_off_delay)
    {
      previousMicros = currentMicros;
      io.digitalWrite(motor_controller_arm_A_pause_pin, LOW);
      io.digitalWrite(motor_controller_arm_B_pause_pin, LOW);
    }    
    outValues[PAUSE_CONTROLLER_ARMS] = 1;
  } 
  else if (start == 0)
  {
    io.digitalWrite(motor_controller_arm_A_pause_pin, LOW);
    io.digitalWrite(motor_controller_arm_B_pause_pin, LOW);
    outValues[PAUSE_CONTROLLER_ARMS] = 0;
  }  
}
void arms_solenoid_safety(int start)
{
  if (start == 1 && outValues[MOVING_CONTROLLER_ARM_A] == 0 && outValues[MOVING_CONTROLLER_ARM_B] == 0)
  {
    modio.setRelay(arm_A_solenoid_safety_pin, 1);
    modio.setRelay(arm_B_solenoid_safety_pin, 1);
    outValues[SOLENOID_CONTROLLER_ARMS] = 1;
  } 
  if (start == 1 && outValues[MOVING_CONTROLLER_ARM_A] == 1 && outValues[MOVING_CONTROLLER_ARM_B] == 1)
  {
      DynamicJsonBuffer  jsonBuffer(200);
      JsonObject& root = jsonBuffer.createObject();
      root["sender"] = hostname;
      root["method"] = "info";
      JsonArray& outputs = root.createNestedArray("outputs");
      JsonObject& outid_val = outputs.createNestedObject();
      outid_val["id"] = outIDs[4];
      outid_val["value"] = outValues[4];
      root["trigger"] = "Arms moving";
      char full_char[250];
      root.prettyPrintTo(full_char, sizeof(full_char));
      pubMsg(full_char);  
  }

  else if (start == 0)
  {
    modio.setRelay(arm_A_solenoid_safety_pin, 0);
    modio.setRelay(arm_B_solenoid_safety_pin, 0);
    outValues[SOLENOID_CONTROLLER_ARMS] = 0;
  }    
}

// void motor_controller_arm_B_start_position(int start){
//   if (start == 1 && rings_move == false)
//   {
//     digitalWrite(motor_controller_arm_B_start_pin, HIGH);
//     outValues[6] = 1;
//   } 
//   else if (start == 0)
//   {
//     digitalWrite(motor_controller_arm_B_start_pin, LOW);
//     outValues[6] = 0;
//   }
// }
// void motor_controller_arm_B_end_position(int start){
//   if (start == 1)
//   {
//     digitalWrite(motor_controller_arm_B_end_pin, HIGH);
//     outValues[7] = 1;
//   } 
//   else if (start == 0)
//   {
//     digitalWrite(motor_controller_arm_B_end_pin, LOW);
//     outValues[7] = 0;
//   }
// }
// void motor_controller_arm_B_enable(int start){
//   if (start == 1)
//   {
//     digitalWrite(motor_controller_arm_B_enable_pin, HIGH);
//     outValues[8] = 1;
//   } 
//   else if (start == 0)
//   {
//     digitalWrite(motor_controller_arm_B_enable_pin, LOW);
//     outValues[8] = 0;
//   }  
// }
// void motor_controller_arm_B_pause(int start){
//   if (start == 1)
//   {
//     digitalWrite(motor_controller_arm_B_pause_pin, HIGH);
//     outValues[9] = 1;
//   } 
//   else if (start == 0)
//   {
//     digitalWrite(motor_controller_arm_B_pause_pin, LOW);
//     outValues[9] = 0;
//   }  
// }
// void arm_B_solenoid_safety(int start){
//   if (start == 1)
//   {
//     digitalWrite(arm_B_solenoid_safety_pin, HIGH);
//     outValues[10] = 1;
//   } 
//   else if (start == 0)
//   {
//     digitalWrite(arm_B_solenoid_safety_pin, LOW);
//     outValues[10] = 0;
//   }    
// }

// rings functions 
void motor_controller_rings_enable(int start){
  if (start == 1)
  {
    io.digitalWrite(motor_controller_rings_enable_pin, HIGH);
    outValues[ENABLE_CONTROLLER_RINGS] = 1;
  } 
  else if (start == 0)
  {
    io.digitalWrite(motor_controller_rings_enable_pin, LOW);
    outValues[ENABLE_CONTROLLER_RINGS] = 0;
  }  
}

void motor_controller_rings_start(int start){
  // if (start == 1 && arm_A_up == true)
  if (start == 1)
  {
    rings_move = true; 
    io.digitalWrite(motor_controller_rings_start_pin, HIGH);
    setOutputWithMessage(START_CONTROLLER_RINGS, 1, "timer");
    if (currentMicros - previousMicros >= motor_controller_on_off_delay)
    {
      previousMicros = currentMicros;
      io.digitalWrite(motor_controller_rings_start_pin, LOW);
    } 
    outValues[START_CONTROLLER_RINGS] = 1;
  } 
  else if (start == 0)
  {
    io.digitalWrite(motor_controller_rings_start_pin, LOW);
    outValues[START_CONTROLLER_RINGS] = 0;
  }
}

void motor_controller_rings_pause(int start){
  if (start == 1)
  {
    io.digitalWrite(motor_controller_rings_pause_pin, HIGH);
    setOutputWithMessage(PAUSE_CONTROLLER_RINGS, 1, "timer");    
    if (currentMicros - previousMicros >= motor_controller_on_off_delay)
    {
      previousMicros = currentMicros;
      io.digitalWrite(motor_controller_rings_pause_pin, LOW);
    }
    outValues[PAUSE_CONTROLLER_RINGS] = 1;
  } 
  else if (start == 0)
  {
    io.digitalWrite(motor_controller_rings_pause_pin, LOW);
    outValues[PAUSE_CONTROLLER_RINGS] = 0;
  }  
}
void motor_controller_rings_resume(int start){
  if (start == 1)
  {
    io.digitalWrite(motor_controller_rings_resume_pin, HIGH);
    setOutputWithMessage(RESUME_CONTROLLER_RINGS, 1, "timer");    
    if (currentMicros - previousMicros >= motor_controller_on_off_delay)
    {
      previousMicros = currentMicros;
      io.digitalWrite(motor_controller_rings_resume_pin, LOW);
    }
    outValues[RESUME_CONTROLLER_RINGS] = 1;
  } 
  else if (start == 0)
  {
    io.digitalWrite(motor_controller_rings_resume_pin, LOW);
    outValues[RESUME_CONTROLLER_RINGS] = 0;
  }    
}
// the led hole function
void led_hole_clb(int start_end)
{
  // increase the LED brightness
  if ( start_end == 1){
    outValues[LED_HOLE] = 1;
    Serial.println("set the led hole state start");
    led_hole_end = false;
    led_hole_begin = true;
  }

  // increase the LED brightness
  if ( start_end == 0){
    outValues[LED_HOLE] = 0;
    Serial.println("set the led hole state end");
    led_hole_begin = false;
    led_hole_end = true;
  }
}



// INPUT FUNCTIONS


// function to read, set and send the inputs on the controller
void digitalReadSetValue(uint8_t name_id)
{
    _input[name_id].current_debounce = millis();
    if (_input[name_id].current_debounce - _input[name_id].last > _input[name_id].debounce)
    {
        if (!digitalRead(_input[name_id].pin) && _input[name_id].value == 0)
        {
            _input[name_id].value = 1;
            Sherlocked.sendInput(name_id, _input[name_id].value, T_INPUT);
            Serial.printf("input %i: 1\n", _input[name_id].pin);
        }
        else if (digitalRead(_input[name_id].pin) && _input[name_id].value == 1)
        {
            _input[name_id].value = 0;
            Sherlocked.sendInput(name_id, _input[name_id].value, T_INPUT);
            Serial.printf("input %i: 0\n", _input[name_id].pin);
        }
        _input[name_id].last = _input[name_id].current_debounce;  
    }
}

// function to read, set and send the inputs on the modio
// void modIOReadSetValue(uint8_t name_id)
// {
//     _input[name_id].current_debounce = millis();
//     if (_input[name_id].current_debounce - _input[name_id].last > _input[name_id].debounce)
//     {
//         if (digitalRead(_input[name_id].pin) && _input[name_id].value == 0) // dit moet gelinkt worden aan de inputState
//         {
//             _input[name_id].value = 1;
//             Sherlocked.sendInput(name_id);
//             // Serial.printf("input %i: 1\n", inputPin);
//         }
//         else if (!digitalRead(_input[name_id].pin) && _input[name_id].value == 1)
//         {
//             _input[name_id].value = 0;
//             Sherlocked.sendInput(name_id);
//             // Serial.printf("input %i: 0\n", inputPin);
//         }
//         _input[name_id].last = _input[name_id].current_debounce;  
//     }
// }



// set the state depending on the output
void outputStateMachine(int id, int value)
{
  id = id -1; 
  dbf("outputStateMachine received id: %i with value: %i\n", id, value);
  if (id == ENABLE_CONTROLLER_ARMS ){
    Serial.printf("ENABLE_CONTROLLER_ARMS OutputStateMachine %i \n", outValues[ENABLE_CONTROLLER_ARMS]);
    motor_controller_arms_enable(value);
  }  
  if (id == TOP_CONTROLLER_ARMS ){
    Serial.println("TOP_CONTROLLER_ARMS");
    Serial.printf("SOLENOID_CONTROLLER_ARMS: %i\n", outValues[SOLENOID_CONTROLLER_ARMS]);
    Serial.printf("START_CONTROLLER_RINGS: %i\n", outValues[START_CONTROLLER_RINGS]);
    motor_controller_arms_top_position(value);    
  }
  if (id == BOTTOM_CONTROLLER_ARMS ){
   Serial.println("BOTTOM_CONTROLLER_ARMS");
   motor_controller_arms_bottom_position(value);
  }
  if (id == PAUSE_CONTROLLER_ARMS ){
    Serial.printf("PAUSE_CONTROLLER_ARMS %i", PAUSE_CONTROLLER_ARMS);
    motor_controller_arms_pause(value);
  }
  if (id == SOLENOID_CONTROLLER_ARMS ){  
    Serial.println("SOLENOID_CONTROLLER_ARMS");
    arms_solenoid_safety(value);
  }  
  if (id == ENABLE_CONTROLLER_RINGS ){
    Serial.println("ENABLE_CONTROLLER_RINGS");
    motor_controller_rings_enable(value);
  }  
  if (id == START_CONTROLLER_RINGS ){
    Serial.println("START_CONTROLLER_RINGS");
    motor_controller_rings_start(value);
  }
  if (id == PAUSE_CONTROLLER_RINGS ){
    Serial.println("PAUSE_CONTROLLER_RINGS");
    motor_controller_rings_pause(value);
  }
  if (id == RESUME_CONTROLLER_RINGS ){
    Serial.println("RESUME_CONTROLLER_RINGS");
    motor_controller_rings_resume(value);
  }  
  if (id == LED_HOLE ){
    Serial.println("LED_HOLE");
    led_hole_clb(value);
  }
}
void inputStateMachine()
{
  digitalReadSetValue(TOP_CONTROLLER_ARM_A);
  digitalReadSetValue(BOTTOM_CONTROLLER_ARM_A);
  digitalReadSetValue(TOP_CONTROLLER_ARM_B);
  digitalReadSetValue(BOTTOM_CONTROLLER_ARM_B);
  digitalReadSetValue(MOVING_CONTROLLER_RINGS);
  digitalReadSetValue(TARGET_REACHED_CONTROLLER_RINGS);
  // digitalReadSetValue(inductive_sensor_A_bottom_pin, 18, &inductive_sensor_A_bottom);
  // digitalReadSetValue(motor_control_A_top_pin, 19, &motor_control_A_up);
  // digitalReadSetValue(motor_control_A_bottom_pin, 19, &motor_control_A_bottom);  
  // digitalReadSetValue(inductive_sensor_B_top_pin, 19, &inductive_sensor_B_top);
  // digitalReadSetValue(inductive_sensor_B_bottom_pin, 19, &inductive_sensor_B_bottom);
  // digitalReadSetValue(motor_control_B_top_pin, 20, &motor_control_B_up);
  // digitalReadSetValue(motor_control_B_bottom_pin, 21, &motor_control_B_bottom);
  // digitalReadSetValue(motor_ring_running_pin, 22, &motor_ring_running);           
}

// functions to set the in and outputs at the right starting position


// functions to send all out- and inputs
void sendAllOutputs()
{
  char * msg = Sherlocked.sendOutputs(NUM_OUTPUTS, outIDs, outValues, T_REQUEST);
  pubMsg(msg);
}
char * getAllOutputs()
{
  char * msg = Sherlocked.sendOutputs(NUM_OUTPUTS, outIDs, outValues, T_REQUEST);
  return(msg);
}
void sendAllInputs()
{
  int allids[NUM_INPUTS]; // actual ids
  int allvals[NUM_INPUTS]; // actual values
  for (int i = 0; i < NUM_INPUTS; i++)
  {
      allids[i]  = _input[i].id;
      allvals[i] = _input[i].value;
  }
  char * msg = Sherlocked.sendInputs(NUM_INPUTS, allids, allvals, T_REQUEST);
  pubMsg(msg);
}
char * getAllInputs()
{
  int allids[NUM_INPUTS]; // actual ids
  int allvals[NUM_INPUTS]; // actual values
  for (int i = 0; i < NUM_INPUTS; i++)
  {
      allids[i]  = _input[i].id;
      allvals[i] = _input[i].value;
  }
  char * msg = Sherlocked.sendInputs(NUM_INPUTS, allids, allvals, T_REQUEST);
  return(msg);
}
// helper functions for getting the right value for the right id
// int getOutputValueByID(int id)
// {
//   for(int i = 0; i < NUM_OUTPUTS; i++)
//   {
//     if(outIDs[i] == id)
//     {
//       return outValues[i];
//     }
//   }
// }
// int getInputValueByID(int id)
// {
//   for(int i = 0; i < NUM_INPUTS; i++)
//   {
//     if(inIDs[i] == id)
//     {
//       return inValues[i];
//     }
//   }
// }
int getOutputArrayIndexByID(int id)
{
  for (int i = 0; i < NUM_OUTPUTS; i++)
  {
    if (outIDs[i] == id)
    {
      return i;
    }
  }
  return UNDEFINED;
}
int getInputArrayIndexByID(int id)
{
  for (int i = 0; i < NUM_INPUTS; i++)
  {
    if (_input[i].id == id)
    {
      return i;
    }
  }
  return UNDEFINED;
}

// puzzle controller specific functions
void resetPuzzle(){
  pubMsg_kb("info", "state", "reset");
};



// the ethernet function
char localIP[16];
char macAddress[18];
void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      ETH.setHostname(hostname);
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      ETH.localIP().toString().toCharArray(localIP, 16);
      strncpy( macAddress, ETH.macAddress().c_str(), sizeof(macAddress) );
      Serial.print("ETH MAC: ");
      Serial.print(macAddress);
      Serial.print(", IPv4: ");
      Serial.print(localIP);
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

// Here starts the ACE/MQTT implementation --- this is rather long
#define dbf Serial.printf
char lastWillMsg[110];
char _incomingMessage[MESSAGE_LENGTH];
uint32_t _lastMqttSend = 0;

void reconnect() {
  sprintf(lastWillMsg, "{\"sender\":\"%s\",\"method\":\"info\",\"state\":\"offline\",\"trigger\":\"disconnect\"}", hostname);
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection to ");
    Serial.print(server);
    Serial.print("...");
    // Attempt to connect
    if (client.connect(hostname, "", "", puzzle_topic, 0, true, lastWillMsg)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      pubMsg_kb("info", "connected", "true", "trigger", "startup");
      // ... and resubscribe
      client.subscribe(gen_topic);
      client.subscribe(puzzle_topic);
      client.subscribe(module_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqttDisconnect(){
  client.disconnect();
}

void callback(char* topic, byte* payload, unsigned int length){
    uint16_t incMsgLen = 0;
    for (int i = 0; i < length; i++)
    {
        char c = (char)payload[i];
        _incomingMessage[incMsgLen] = c;
        incMsgLen ++;
    }
    _incomingMessage[incMsgLen] = '\0'; // add 0 at the end for str functions
    incMsgLen ++;
    Sherlocked.parse(_incomingMessage);
}



/* Always keep track of the puzzle state locally */
uint8_t _state = S_IDLE;

void setState(uint8_t s, int trigger)
{
  _state = s;
  dbf("State now %s (%u), triggered by %s\n", Sherlocked.getStateStr(s), s, Sherlocked.getTriggerStr(trigger));
}

uint8_t getState()
{
  return _state;
}

/* State callback is triggered whenever a 'state' is received */
void stateCallback(int meth, int state, int trigger)
{
  if (meth == M_PUT)
  {
    setState(state, trigger);
  }
  else if (meth == M_GET)
  {
    pubMsg(Sherlocked.sendState(getState(), T_REQUEST));
  }
}

/* input callback is triggered whenever 'inputs' is received; this is always an array */
void inputCallback(int meth, int numInputs, int ids[], int vals[], int trigger)
{
  dbf("inputCallback() method : %s, numInputs : %i, trigger : %s\n", Sherlocked.getMethodStr(meth), numInputs, Sherlocked.getTriggerStr(trigger) );
  if (meth == M_GET)
  {
    // The get method only has ids; fill in the values with the values that correspond to the input ID
    if (numInputs > 0)
    {
      int numOnBoard = 0;   // count the actual number on this board
      int aids[numInputs]; // actual ids
      int avals[numInputs]; // actual values
      for (int i = 0; i < numInputs; i++)
      {
        int idx = getInputArrayIndexByID(ids[i]); // find the array index of the output id on this board
        if (idx != UNDEFINED) // if found store it
        {
          aids[numOnBoard]  = _input[idx].id;
          avals[numOnBoard] = _input[idx].value;
          numOnBoard++;
        }
      }
      char * msg = Sherlocked.sendInputs(numOnBoard, aids, avals, T_REQUEST);
      pubMsg(msg); 
    }
    else  // If no input id's are provided, feed them all back
    {
      sendAllInputs();
    }
  }
  else if (meth == M_INFO) // listen to other input info
  {
    for (int i = 0; i < numInputs; i++)
    {
      // possibly do something with info received from other controllers within your own puzzle
    }
  }
}

/* output callback is triggered whenever 'outputs' is received; this is always an array */
void outputCallback(int meth, int numOutputs, int ids[], int vals[], int trigger)
{
  dbf("outputCallback() method : %s, numOutputs : %i, trigger : %s\n", Sherlocked.getMethodStr(meth), numOutputs, Sherlocked.getTriggerStr(trigger) );
  if (meth == M_PUT)
  {
    // set all desired outputs
    for (int i = 0; i < numOutputs; i++)
    {
      // set value SET HERE YOUR STATE OR START YOUR ACTION NEEDED.
      // dbf("\timplement: setOutput(int id: %i, int val: %i)\n", ids[i], vals[i]);
      outputStateMachine(ids[i], vals[i]);
    }

    // And compile a reply to let the server know the new outputs state
    int numOnBoard = 0;   // count the actual number on this board
    int aids[numOutputs]; // actual ids
    int avals[numOutputs]; // actual values
    for (int i = 0; i < numOutputs; i++)
    {
      int idx = getOutputArrayIndexByID(ids[i]); // find the array index of the output id on this board
      if (idx != UNDEFINED) // if found store it
      {
        aids[numOnBoard]  = outIDs[idx];
        avals[numOnBoard] = outValues[idx];
        numOnBoard++;
      }
    }
    if (numOnBoard > 0)
    {
      char * msg = Sherlocked.sendOutputs(numOnBoard, aids, avals, T_REQUEST);
      pubMsg(msg);
    }
  }
  else if (meth == M_GET)
  {
    // The get method only has ids; fill in the values with the values that correspond to the output ID
    if (numOutputs > 0)
    {
      int numOnBoard = 0;   // count the actual number on this board
      int aids[numOutputs]; // actual ids
      int avals[numOutputs]; // actual values
      for (int i = 0; i < numOutputs; i++)
      {
        int idx = getOutputArrayIndexByID(ids[i]); // find the array index of the output id on this board
        if (idx != UNDEFINED) // if found store it
        {
          aids[numOnBoard]  = outIDs[idx];
          avals[numOnBoard] = outValues[idx];
          numOnBoard++;
        }
      }
      if (numOnBoard > 0)
      {
        char * msg = Sherlocked.sendOutputs(numOnBoard, aids, avals, T_REQUEST);
        pubMsg(msg);
      }
    }
    else  // If no output id's are provided, feed them all back
    {
      pubMsg(Sherlocked.sendOutputs(NUM_OUTPUTS, outIDs, outValues, T_REQUEST));
    }
  }
  else if (meth == M_INFO) // listen to other output info
  {
    for (int i = 0; i < numOutputs; i++)
    {
      // possibly do something with info received from other controllers within your own puzzle
    }
  }
}

/*  The JSON Handler is optional. This handler can be used for custom JSON messages that are not covered by the MessageHandler
    This function provides a JSON object from the ArduinoJson Library
 */
void jsonCallback(JsonObject & json)
{
  dbf("jsonCallback() : ");
  json.printTo(Serial);

  if (json.containsKey("r") && json.containsKey("g") && json.containsKey("b"))
  {
    // Extract the values from the JSON object
    //  uint8_t r = json["r"];
    //  uint8_t g = json["g"];
    //  uint8_t b = json["b"];
     // And use it in a suitable function
     // setLEDColor(r, g, b); 
  }
  // Or use it to turn a motor in a certain direction
  // {"direction":"left"}
  else if(json.containsKey("direction")) 
  {
    // Extract the value from the JSON object
    const char * pos = json["direction"];
    // Compare the value with expected values
    if (strcmp(pos, "left") == 0)
    {
      // turnMotorLeft();
    } 
    else if (strcmp(pos, "right") == 0)
    {
      // turnMotorRight();
    } 
  }
  
}

// end of the ACE/MQTT implementation



// START OF THE OTA CODE
#include <Update.h>
uint16_t _otaTimeout = 15000;
int contentLength = 0;
bool isValidContentType = false;
String host = server.toString();
int port = 80; // Non https. For HTTPS 443.  HTTPS doesn't work yet
String bin; // bin file name with a slash in front.

void setBinVers(const char binfile[])
{
  // bin = "/ota/";
  String bf = String(binfile);
  bin = bf;
  Serial.print("Setting bin file to: ");
  Serial.println(bin);
}

// Utility to extract header value from headers
String getHeaderValue(String header, String headerName)
{
  return header.substring(strlen(headerName.c_str()));
}

// OTA Logic
void execOTA()
{
  // otaClient.setTimeout(_otaTimeout);
  Serial.println("Connecting to: " + String(host));
  if (ethclient.connect(host.c_str(), port)) {
    // Connection Succeed.
    Serial.println("Fetching Bin: " + String(bin));
    // Get the contents of the bin file
    ethclient.print(String("GET ") + bin + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "Cache-Control: no-cache\r\n" +
                    "Connection: close\r\n\r\n");

    // Check what is being sent
    //    Serial.print(String("GET ") + bin + " HTTP/1.1\r\n" +
    //                 "Host: " + host + "\r\n" +
    //                 "Cache-Control: no-cache\r\n" +
    //                 "Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    while (ethclient.available() == 0) {
      if (millis() - timeout > _otaTimeout) {
        Serial.println("Client Timeout !");
        ethclient.stop();
        return;
      }
    }
    // Once the response is available,
    // check stuff

    /*
       Response Structure
        HTTP/1.1 200 OK
        x-amz-id-2: NVKxnU1aIQMmpGKhSwpCBh8y2JPbak18QLIfE+OiUDOos+7UftZKjtCFqrwsGOZRN5Zee0jpTd0=
        x-amz-request-id: 2D56B47560B764EC
        Date: Wed, 14 Jun 2017 03:33:59 GMT
        Last-Modified: Fri, 02 Jun 2017 14:50:11 GMT
        ETag: "d2afebbaaebc38cd669ce36727152af9"
        Accept-Ranges: bytes
        Content-Type: application/octet-stream
        Content-Length: 357280
        Server: AmazonS3

        {{BIN FILE CONTENTS}}

    */
    while (ethclient.available()) {
      // read line till /n
      String line = ethclient.readStringUntil('\n');
      // remove space, to check if the line is end of headers
      line.trim();

      // if the the line is empty,
      // this is end of headers
      // break the while and feed the
      // remaining `ethclient` to the
      // Update.writeStream();
      if (!line.length()) {
        //headers ended
        break; // and get the OTA started
      }

      // Check if the HTTP Response is 200
      // else break and Exit Update
      if (line.startsWith("HTTP/1.1")) {
        if (line.indexOf("200") < 0) {
          Serial.println("Got a non 200 status code from server. Exiting OTA Update.");
          break;
        }
      }

      // extract headers here
      // Start with content length
      if (line.startsWith("Content-Length: ")) {
        contentLength = atoi((getHeaderValue(line, "Content-Length: ")).c_str());
        Serial.println("Got " + String(contentLength) + " bytes from server");
      }

      // Next, the content type
      if (line.startsWith("Content-Type: ")) {
        String contentType = getHeaderValue(line, "Content-Type: ");
        Serial.println("Got " + contentType + " payload.");
        if (contentType == "application/octet-stream") {
          isValidContentType = true;
        }
      }
    }
  } else {
    // Connect to failed
    // May be try?
    // Probably a choppy network?
    Serial.println("Connection to " + String(host) + " failed. Please check your setup");
    // retry??
    // execOTA();
  }

  // Check what is the contentLength and if content type is `application/octet-stream`
  Serial.println("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));

  // check contentLength and content type
  if (contentLength && isValidContentType) {
    // Check if there is enough to OTA Update
    bool canBegin = Update.begin(contentLength);

    // If yes, begin
    if (canBegin) {
      Serial.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!");
      // No activity would appear on the Serial monitor
      // So be patient. This may take 2 - 5mins to complete
      size_t written = Update.writeStream(ethclient);

      if (written == contentLength) {
        Serial.println("Written : " + String(written) + " successfully");
      } else {
        Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?" );
        // retry??
        // execOTA();
      }

      if (Update.end()) {
        Serial.println("OTA done!");
        if (Update.isFinished()) {
          Serial.println("Update successfully completed. Rebooting.");
          ESP.restart();
        } else {
          Serial.println("Update not finished? Something went wrong!");
        }
      } else {
        Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }
    } else {
      // not enough space to begin OTA
      // Understand the partitions and
      // space availability
      Serial.println("Not enough space to begin OTA");
      ethclient.flush();
    }
  } else {
    Serial.println("There was no content in the response");
    ethclient.flush();
  }
}
void startOTA()
{
  dbf("StartOTA\n");
  if (bin != NULL && !bin.equals(""))
  {
    char temp [MESSAGE_LENGTH];
    bin.toCharArray(temp, bin.length() + 1);
    dbf("Bin is set to %s\n", temp);
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi is down...");
      WiFi.onEvent(WiFiEvent);
      ETH.begin();
    }
    // while (WiFi.status() == WL_CONNECTED) {
    //   delay(1);  // wait for a bit
    // }
    char ota_info[200];
    sprintf(ota_info, "{ \"event\": \"OTA\", \"URI\": \"%s\", \"current_firmware\": \"%s\" }", temp, firmware_version);
    pubMsg_kb("info", "info", ota_info, "trigger", "disconnect");
    delay(100);
    mqttDisconnect();
    execOTA();
  }
  else
  {
    dbf("No file for OTA, check for updates?\n");
//    checkForUpdates();
  }
}
void doOTA(const char binfile[])
{
  setBinVers(binfile);
  startOTA();
}
// END OF THE OTA CODE

// The function that actually does the incoming commands
/* The commandCallback function is called (activated) when a new 'cmd' or 'info' command is received */
void commandCallback(int meth, int cmd, const char value[], int triggerID)
{
  switch (cmd)
  {
    case CMD_RESET:
      dbf("Received Puzzle RESET from Sherlocked\n");
      resetPuzzle();
      break;

    case CMD_REBOOT:
      dbf("Received Reboot from Sherlocked\n");
      ESP.restart();
      break;

    case CMD_SYNC:
      dbf("Sync Command not implemented for this board\n");  // en is ook niet nodig, alleen voor files belangrijk
      break;

    case CMD_OTA:
      dbf("OTA Firmware update command\n"); // deze gaan we wel doen, Serge stuurt me voorbeeld code
      // value contains the file URL for the OTA command
      doOTA(value);
      break;

    case INFO_SYSTEM:
      dbf("system info requested\n");
      char system[200];
      sprintf(system, "{ \"ip\": \"%s\", \"MAC\": \"%s\", \"firmware\": \"%s\" }", localIP, macAddress, firmware_version);
      pubMsg_kb("info", "info", system, "trigger", "request");
        // Expects to receive back system info such as local IP ('ip'), Mac address ('mac') and Firmware Version ('fw')
      break;

    case INFO_STATE:
      dbf("state requested\n");
      pubMsg(Sherlocked.sendState(getState(), T_REQUEST));
      break;

    case INFO_FULLSTATE:
      dbf("full state requested\n");
      // {"sender":"controller","state":"idle","connected":true,"inputs":[{"id":1,"value":1}],"outputs":[{"id":1,"value":0},{"id":3,"value":1}],"method":"info","trigger":"request"}
      DynamicJsonBuffer  jsonBuffer(400);
      JsonObject& root = jsonBuffer.createObject();
      root["sender"] = hostname;
      root["state"] = getState();
      root["connected"] = client.connected();
      JsonArray& inputs = root.createNestedArray("inputs");
      for (int i = 0; i < NUM_INPUTS; i++)
      {
        JsonObject& ind_val = inputs.createNestedObject();
        ind_val["id"] = _input[i].id;
        ind_val["value"] = _input[i].value;
      }
      JsonArray& outputs = root.createNestedArray("outputs");
      for (int i = 0; i < NUM_OUTPUTS; i++)
      {
        JsonObject& ond_val = outputs.createNestedObject();
        ond_val["id"] = outIDs[i];
        ond_val["value"] = outValues[i];
      }
      root["trigger"] = "request";
      char full_char[1200];
      root.prettyPrintTo(full_char, sizeof(full_char));
      pubMsg(full_char);


      // sprintf(full_state, " \"state\":\"%s\", \"connected\": \"%s\", \"inputs\":[{\"id\":1,\"value\":1}] ", getState(), client.connected() );
      // pubMsg_kb("info", "info", full_state, "trigger", "request");
      // Expects to receive back a full state with all relevant inputs and outputs
      break;
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println("Conductive MQTT Sensor");

  initInputs();


  setOutputsNum();

  // pwm output setup for led_hole
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(led_hole_pin, ledChannel);

  modio.begin();

  Wire1.begin(5, 15, 400000U);
  if (io.begin(SX1509_ADDRESS, Wire1) == false)
  {
    Serial.println("Failed to communicate. Check wiring and address of SX1509.");
    DynamicJsonBuffer  jsonBuffer(200);
    JsonObject& root = jsonBuffer.createObject();
    root["sender"] = hostname;
    root["method"] = "info";
    root["trigger"] = "electronics failure at SX1509";
    char full_char[250];
    root.prettyPrintTo(full_char, sizeof(full_char));
    pubMsg(full_char);  
  }
  else if (io.begin(SX1509_ADDRESS, Wire1) == true)
  {
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
  



  // start the ethernet client
  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  if (eth_connected){
      delay(5000); // wait for all the network things to resolve
      Serial.println("Ethernet Connected");
  }
  // start the mqtt client
  client.setServer(server, 1883);
  client.setCallback(callback);
  client.setBufferSize(1200);

  // Allow the hardware to sort itself out
  delay(1500);

  /* Set the name for this controller, this should be unqiue within */
  Sherlocked.setName(hostname);
  /* Set callback functions for the various messages that can be received by the Sherlocked ACE system */    
  /* Puzzle State Changes are handled here */
  Sherlocked.setStateCallback(stateCallback);
  /* General Command and Info Messages */
  Sherlocked.setCommandCallback(commandCallback);
  /* Inputs and Outputs */
  Sherlocked.setInputCallback(inputCallback);
  Sherlocked.setOutputCallback(outputCallback);
  /* Catch-all callback for json messages that were not handled by other callbacks */
  Sherlocked.setJSONCallback(jsonCallback);

}





void loop() {

  // if the mqtt client does not connect, try it again later
  if (!client.connected()) {
    reconnect();
  }
  client.loop();  

  // setup for the non blocking functions
  currentMicros = micros();

  inputStateMachine();

  // Led animations 
  // increase the LED HOLE brightness
  if ( led_hole_begin == true && dutyCycle <= 4096)
  {
    if (currentMicros - previousMicros >= led_hole_fade_interval) 
    {
      previousMicros = currentMicros;   
      dutyCycle++;
      // Serial.println(dutyCycle);
      ledcWrite(ledChannel, dutyCycle);
      if (dutyCycle == 4096){
        led_hole_begin = false;
        Serial.println("led_hole fade in done");
      }
    } 
  }
  // decrease the brightness
  if (led_hole_end == true && dutyCycle >= 0)
  {
    if (currentMicros - previousMicros >= led_hole_fade_interval) 
    {
      previousMicros = currentMicros;   
      dutyCycle--;
      // Serial.println(dutyCycle);
      ledcWrite(ledChannel, dutyCycle);
      if (dutyCycle == 0){
        led_hole_end = false;
        Serial.println("led_hole fade out done");
      }
    } 
  }
  
}