// the firmware for the pole sensor boards for the Alchemist Centrepiece 

#include <Arduino.h>
#include <SPI.h>
#include <ETH.h>
#include <PubSubClient.h>
#include <Sherlocked.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>



// Firmware version
char firmware_version[] = "0.5";                                // now we support outputCallback M = M_Put!  

// SETTINGS

// mqtt/ace settings
char hostname[] ="controller";                                  // the hostname for board  <= REPLACE
const char gen_topic[] = "alch";  
const char puzzle_topic[] = "alch/centrepiece"; 
const char module_topic[] = "alch/centrepiece/controller";      // the module name of the board <= REPLACE
IPAddress server(192, 168, 178, 214);                           // ip address of the mqtt/ace server <= REPLACE


// controller settings

//in- and outputs --- these needs to correspond to the document at https://docs.google.com/document/d/1GiCjMT_ph-NuIOsD4InIvT-H3MmUkSkzBZRMM1L5IsI/edit#heading=h.wqfd6v7o79qu
#define NUM_OUTPUTS 17          // amount of outputs
#define START_OUTPUT 1          // the start number of the output
#define NUM_INPUTS 4            // amount of inputs
#define START_INPUT 17          // the start number of the input

//pwm 
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 12;

//led ring 
uint16_t loop_time = 1;         // the time it costs to loop over a ring
uint16_t TailLength = 80;       // length of the tail, must be shorter than PixelCount
uint16_t PixelCount = 720;      // make sure to set this to the number of pixels in your strip
float MaxLightness = 0.2f;       // max lightness at the head of the tail (0.5f is full bright)
float hue = 0.16667f;            // hue scales from 0.0f to 1.0f
float saturation = 0.05f;         // saturation scales from 0.0f to 1.0f with 0.0f no saturation


const uint16_t AnimCount = 1;           // we only need one
const uint16_t timeScale = NEO_MILLISECONDS; // make sure it's set at milliseconds, although it is the default



// PIN ASSIGNMENT
const int motor_controller_arm_A_start_pin = 13;
const int motor_controller_arm_A_end_pin = 14;
const int motor_controller_arm_A_enable_pin = 15;
const int arm_A_solenoid_safety_pin = 16;
const int motor_controller_arm_B_start_pin = 32;
const int motor_controller_arm_B_end_pin = 33;
const int motor_controller_arm_B_enable_pin = 34;
const int arm_B_solenoid_safety_pin = 35;
const int motor_controller_rings_start_pin = 36;
const int motor_controller_rings_enable_pin = 39;
const int led_ring_1_pin = 2;
const int led_ring_2_pin = 3;
const int led_ring_3_pin = 4;
const int led_hole_pin = 5;



// SETUP LIBS
WiFiClient ethclient;
PubSubClient client(ethclient);
NeoGamma<NeoGammaTableMethod> colorGamma; 
NeoPixelBus<NeoGrbwFeature, NeoSk6812Method> ring1(PixelCount, led_ring_1_pin);
NeoPixelBus<NeoGrbwFeature, NeoSk6812Method> ring2(PixelCount, led_ring_2_pin);
NeoPixelBus<NeoGrbwFeature, NeoSk6812Method> ring3(PixelCount, led_ring_3_pin);
// NeoPixel animation management object
NeoPixelAnimator animations(AnimCount, timeScale ); 


// variables 
int outIDs[NUM_OUTPUTS];                    // array to store the id's of the outputs
int outValues[NUM_OUTPUTS] = {0};           // array to store the values of the outputs, initialized with all set to 0
int inIDs[NUM_INPUTS];
int inValues[NUM_INPUTS] = {0};
static bool eth_connected = false;
unsigned long previousMicros = 0;
int long led_hole_fade_interval = 500;
bool led_hole_begin = false; 
bool led_hole_end = false; 
bool led_rings_start = false;
bool rings_move = false;
bool arms_up = false; 
RgbwColor black(0);
int dutyCycle = 0; 


// CONTROLLER SPECIFIC FUNCTIONS

// the led_ring functions
void LoopAnimUpdate(const AnimationParam& param)
{
    // wait for this animation to complete,
    // we are using it as a timer of sorts
    if (param.state == AnimationState_Completed)
    {
        // done, time to restart this position tracking animation/timer
        animations.RestartAnimation(param.index);

        // rotate the complete strip one pixel to the right on every update
        ring1.RotateRight(8);
    }
}
void DrawTailPixels()
{
    // using Hsl as it makes it easy to pick from similiar saturated colors
    // float hue = 1.0f;
    for (uint16_t index = 0; index < ring1.PixelCount() && index <= TailLength; index++)
    {
        float lightness = index * MaxLightness / TailLength;
        RgbwColor color = HslColor(hue, saturation, lightness);

        ring1.SetPixelColor(index, colorGamma.Correct(color));
    }
}

// the motor functions
void motor_controller_arm_A_start_position(int start){
  if (start == 1 && rings_move == false)
  {
    digitalWrite(motor_controller_arm_A_start_pin, HIGH);
    outValues[1] = 1;
  } 
  else 
  {
    digitalWrite(motor_controller_arm_A_start_pin, LOW);
    outValues[1] = 0;
  }
}
void motor_controller_arm_A_end_position(int start){
  if (start == 1)
  {
    digitalWrite(motor_controller_arm_A_end_pin, HIGH);
    outValues[2] = 1;
  } 
  else 
  {
    digitalWrite(motor_controller_arm_A_end_pin, LOW);
    outValues[2] = 0;
  }
}
void motor_controller_arm_A_enable(int start){
  if (start == 1)
  {
    digitalWrite(arm_A_solenoid_safety_pin, HIGH);
    outValues[3] = 1;
  } 
  else 
  {
    digitalWrite(arm_A_solenoid_safety_pin, LOW);
    outValues[3] = 0;
  }  
}
void arm_A_solenoid_safety(int start){
  if (start == 1)
  {
    digitalWrite(arm_A_solenoid_safety_pin, HIGH);
    outValues[4] = 1;
  } 
  else 
  {
    digitalWrite(arm_A_solenoid_safety_pin, LOW);
    outValues[4] = 0;
  }    
}

void motor_controller_arm_B_start_position(int start){
  if (start == 1 && rings_move == false)
  {
    digitalWrite(motor_controller_arm_B_start_pin, HIGH);
    outValues[5] = 1;
  } 
  else 
  {
    digitalWrite(motor_controller_arm_B_start_pin, LOW);
    outValues[5] = 0;
  }
}
void motor_controller_arm_B_end_position(int start){
  if (start == 1)
  {
    digitalWrite(motor_controller_arm_B_end_pin, HIGH);
    outValues[6] = 1;
  } 
  else 
  {
    digitalWrite(motor_controller_arm_B_end_pin, LOW);
    outValues[6] = 0;
  }
}
void motor_controller_arm_B_enable(int start){
  if (start == 1)
  {
    digitalWrite(motor_controller_arm_B_enable_pin, HIGH);
    outValues[7] = 1;
  } 
  else 
  {
    digitalWrite(motor_controller_arm_B_enable_pin, LOW);
    outValues[7] = 0;
  }  
}
void arm_B_solenoid_safety(int start){
  if (start == 1)
  {
    digitalWrite(arm_B_solenoid_safety_pin, HIGH);
    outValues[8] = 1;
  } 
  else 
  {
    digitalWrite(arm_B_solenoid_safety_pin, LOW);
    outValues[8] = 0;
  }    
}

void motor_controller_rings_start(int start){
  if (start == 1 && arms_up == true)
  {
    rings_move = true; 
    digitalWrite(motor_controller_rings_start_pin, HIGH);
    outValues[9] = 1;
  } 
  else 
  {
    digitalWrite(motor_controller_rings_start_pin, LOW);
    outValues[9] = 0;
  }
}
void motor_controller_rings_enable(int start){
  if (start == 1)
  {
    digitalWrite(motor_controller_rings_enable_pin, HIGH);
    outValues[10] = 1;
  } 
  else 
  {
    digitalWrite(motor_controller_rings_enable_pin, LOW);
    outValues[10] = 0;
  }  
}

// the ring led function
void led_rings(int start){
  Serial.print("start led_rings: ");
  Serial.println(start);
  if (start == 1)
  {
    led_rings_start = true;
    outValues[11] = 1;
    // Draw the tail that will be rotated through all the rest of the pixels
    DrawTailPixels();    
    animations.StartAnimation(0, loop_time, LoopAnimUpdate);    
  } 
  else 
  {
    led_rings_start = false;
    animations.StopAnimation(0);
    ring1.ClearTo(black);
    ring1.Show();
    outValues[11] = 0;
  }    
}
void led_rings_brightness(int brightness){

  outValues[12] = brightness;
  MaxLightness = (float)brightness / (float)100;
  dbf("led rings brightness: %f \n", MaxLightness);
  animations.StopAnimation(0);
  ring1.ClearTo(black);
  DrawTailPixels();
  animations.StartAnimation(0, loop_time, LoopAnimUpdate);
}
void led_rings_tail(int tail){
  outValues[13] = tail;
  TailLength = tail;
  dbf("led rings tail: %i \n", TailLength);
  animations.StopAnimation(0);
  ring1.ClearTo(black);
  DrawTailPixels();
  animations.StartAnimation(0, loop_time, LoopAnimUpdate);
}
void led_rings_speed(int speed){
  outValues[14] = speed;
  loop_time = speed;
  dbf("led rings speed: %i \n", loop_time);
  animations.StopAnimation(0);
  ring1.ClearTo(black);
  DrawTailPixels();
  animations.StartAnimation(0, loop_time, LoopAnimUpdate);
}
void led_rings_hue(int hue_val){
  outValues[15] = hue_val;
  hue = (float)hue_val / (float)100;
  dbf("led rings hue: %f \n", hue);
  animations.StopAnimation(0);
  ring1.ClearTo(black);
  DrawTailPixels();
  animations.StartAnimation(0, loop_time, LoopAnimUpdate);
}
void led_rings_saturation(int sat_val){
  outValues[16] = sat_val;
  saturation = (float)sat_val / (float)100;
  dbf("led rings saturation: %f \n", saturation);
  animations.StopAnimation(0);
  ring1.ClearTo(black);
  DrawTailPixels();
  animations.StartAnimation(0, loop_time, LoopAnimUpdate);
}

// the led hole function
void led_hole(int start_end)
{
  // increase the LED brightness
  if ( start_end == 1){
    outValues[17] = 1;
    Serial.println("set the led hole state start");
    led_hole_end = false;
    led_hole_begin = true;
  }

  // increase the LED brightness
  if ( start_end == 0){
    outValues[17] = 0;
    Serial.println("set the led hole state end");
    led_hole_begin = false;
    led_hole_end = true;
  }
}

// set the state depending on the output
void outputStateMachine(int id, int value)
{
  Serial.printf("outputStateMachine received id: %i with value: %i\n", id, value);
  if (id == 1 ){
    motor_controller_arm_A_start_position(value);    
  }
  if (id == 2 ){
    motor_controller_arm_A_end_position(value);
  }
  if (id == 3 ){
    motor_controller_arm_A_enable(value);
  }
  if (id == 4 ){
    arm_A_solenoid_safety(value);
  }  
  if (id == 5 ){
    motor_controller_arm_B_start_position(value);
  }
  if (id == 6 ){
    motor_controller_arm_B_end_position(value);
  }
  if (id == 7 ){
    motor_controller_arm_B_enable(value);
  }
  if (id == 8 ){
    arm_B_solenoid_safety(value);
  }
  if (id == 9 ){
    motor_controller_rings_start(value);
  }              
  if (id == 10 ){
    motor_controller_rings_enable(value);
  }
  if (id == 11 ){
    led_rings(value);
  }    
  if (id == 12 ){
    led_rings_brightness(value);
  }  
  if (id == 13 ){
    led_rings_tail(value);
  }   
  if (id == 14 ){
    led_rings_speed(value);
  }   
  if (id == 15 ){
    led_rings_hue(value);
  }  
  if (id == 16 ){
    led_rings_saturation(value);
  }             
  if (id == 17 ){
    led_hole(value);
  }
}

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

// functions to set the in and outputs at the right starting position
void setOutputsNum()
{
  for(int i = 0; i < NUM_OUTPUTS; i++)
  {
    outIDs[i] = i+START_OUTPUT;
  }
}
void setInputsNum()
{
  for(int i = 0; i < NUM_INPUTS; i++)
  {
    inIDs[i] = i+START_INPUT;
  }
}

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
  char * msg = Sherlocked.sendInputs(NUM_INPUTS, inIDs, inValues, T_REQUEST);
  pubMsg(msg);
}
char * getAllInputs()
{
  char * msg = Sherlocked.sendInputs(NUM_INPUTS, inIDs, inValues, T_REQUEST);
  return(msg);
}
// helper functions for getting the right value for the right id
int getOutputValueByID(int id)
{
  for(int i = 0; i < NUM_OUTPUTS; i++)
  {
    if(outIDs[i] == id)
    {
      return outValues[i];
    }
  }
}
int getInputValueByID(int id)
{
  for(int i = 0; i < NUM_INPUTS; i++)
  {
    if(inIDs[i] == id)
    {
      return inValues[i];
    }
  }
}
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
    if (inIDs[i] == id)
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
          aids[numOnBoard]  = inIDs[idx];
          avals[numOnBoard] = inValues[idx];
          numOnBoard++;
        }
      }
      char * msg = Sherlocked.sendInputs(numOnBoard, aids, avals, T_REQUEST);
      pubMsg(msg); 
    }
    else  // If no input id's are provided, feed them all back
    {
      char * msg = Sherlocked.sendInputs(NUM_INPUTS, inIDs, inValues, T_REQUEST);
      pubMsg(msg);
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
      dbf("\timplement: setOutput(int id: %i, int val: %i)\n", ids[i], vals[i]);
      outputStateMachine(ids[i], vals[i]);
    }

    // And compile a reply to let the server know the new outputs state
    int numFBOutputs = 0;
    int fbids[numOutputs];
    int fbvals[numOutputs];
    for (int i = 0; i < numOutputs; i++)
    {
      // Make sure to filter out any ID's that are not on this controller
      // int val = getValueByOutputID(ids[i]);
      int val = vals[i];
      if (val != UNDEFINED)
      {
        fbids[numFBOutputs]   = ids[i];
        fbvals[numFBOutputs]  = val;
        numFBOutputs ++;
      }
    }
    if (numFBOutputs > 0)
    {
      pubMsg(Sherlocked.sendOutputs(numFBOutputs, fbids, fbvals, trigger));
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
      char * msg = Sherlocked.sendOutputs(numOnBoard, aids, avals, T_REQUEST);
      pubMsg(msg);    }
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
        ind_val["id"] = inIDs[i];
        ind_val["value"] = inValues[i];
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

  setOutputsNum();
  setInputsNum();

  // pwm output setup for led_hole
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(led_hole_pin, ledChannel);

  ring1.Begin();
  ring1.ClearTo(black);
  ring1.Show();

  // we use the index 0 animation to time how often we rotate all the pixels
     

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
  unsigned long currentMicros = micros();

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
  
  // led ring animation start
  if (led_rings_start == true){
    // Serial.println("led_rings started");
    animations.UpdateAnimations();
    ring1.Show();
  }  

}