/*
  Example demonstrating the use of the Sherlocked library
*/

/* Include the library for communciation */
#include <Sherlocked.h>

/* The commandCallback function is called (activated) when a new 'cmd' or 'info' command is received */
void commandCallback(int meth, int cmd, const char * value, int triggerID)
{
  switch (cmd)
  {
    case CMD_RESET:
      dbf("Received Puzzle RESET from Sherlocked\n");
//      resetPuzzle();
      break;

    case CMD_REBOOT:
      dbf("Received Reboot from Sherlocked\n");
//      ESP.restart();
      break;

    case CMD_SYNC:
      dbf("Sync Command not implemented for this board\n");
      break;

    case CMD_OTA:
    dbf("OTA Firmware update command\n");
      // value contains the file URL for the OTA command
      // doOTA(value);
      break;

    case INFO_SYSTEM:
        // Expects to receive back system info such as local IP ('ip'), Mac address ('mac') and Firmware Version ('fw')
      break;

    case INFO_STATE:
        pubMsg(Sherlocked.sendState(getState(), T_REQUEST));
      break;

    case INFO_FULLSTATE:
      // Expects to receive back a full state with all relevant inputs and outputs
      break;
  }
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
      for (int i = 0; i < numInputs; i++)
      {
        vals[i] = random(0, 8);
      }
      char * msg = Sherlocked.sendInputs(numInputs, ids, vals, T_REQUEST);
      pubMsg(msg);
    }
    else  // If no input id's are provided, feed them all back
    {
      numInputs = 10;
      // store ids and values in new arrays so we have sufficient space
      int aids[numInputs];
      int avals[numInputs];
      for (int i = 0; i < numInputs; i++)
      {
        aids[i]   =  i + 1;
        avals[i]  =  random(0,255);
      }
      char * msg = Sherlocked.sendInputs(numInputs, aids, avals, T_REQUEST);
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
      // set value
      dbf("\timplement: setOutput(int id: %i, int val: %i)\n", ids[i], vals[i]);
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
      for (int i = 0; i < numOutputs; i++)
      {
        vals[i] = random(0, 8);
      }
      pubMsg(Sherlocked.sendOutputs(numOutputs, ids, vals, T_REQUEST));
    }
    else  // If no output id's are provided, feed them all back
    {
      numOutputs = 10;
      // store ids and values in new arrays so we have sufficient space
      int allids[numOutputs];
      int allvals[numOutputs];
      for (int i = 0; i < numOutputs; i++)
      {
        allids[i]   =  i + 1;
        allvals[i]  =  random(0,255);
      }
      pubMsg(Sherlocked.sendOutputs(numOutputs, allids, allvals, T_REQUEST));
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
     uint8_t r = json["r"];
     uint8_t g = json["g"];
     uint8_t b = json["b"];
     // And use it in a suitable function
     // setLEDColor(r, g, b); 
  }
  // Or use it to turn a motor in a certain direction
  // {"direction":"left"}
  else if(json.containsKey("direction")) 
  {
    // Extract the value from the JSON object
    const char* pos = json["direction"];
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

/*  Phony publish function; replace this with an actual MQTT pub function
    Make sure to publish in the 
*/
void pubMsg(char * msg)
{
  Serial.println(msg);
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

void setup() 
{
  /* Set the name for this controller, this should be unqiue within */
  Sherlocked.setName("controller");

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
  
  /*
   * You can now do the setup of the actual puzzle elements
   * For instance instantiate your debug Serial.
   */

  Serial.begin(115200);
  Serial.println("Start Puzzle Setup"); 

  // When the setup is complete, let the ACE know that the Puzzle is ready
  pubMsg(Sherlocked.sendState(S_ACTIVE, T_STARTUP));
}

void loop() 
{   

  /* Fake Incoming MQTT messages through the serial terminal for testing:
    {"sender":"server", "method":"put", "outputs":[{"id":2,"value":1}]}
    {"sender":"server", "method":"put", "state":"solved"}
    {"sender":"server", "method":"get", "inputs":[2, 4]}
    
   */
  if(Serial.available())
  {
    String incomingMqttMessage = Serial.readStringUntil('\n');
    // The parser accepts a char * (c string)
    char msgBuf[1024];
    strcpy(msgBuf, incomingMqttMessage.c_str());
    Sherlocked.parse(msgBuf);
  }
  
  /* Keep the server up to date about the puzzle state, if its is ready for use, currently in use, or when it is solved 
   *  Sherlocked.sendState(S_IDLE,   T_STATE);
   *  Sherlocked.sendState(S_ACTIVE, T_STATE);
   *  Sherlocked.sendState(S_SOLVED, T_STATE);
   */


  
  /* Keep server up to date about input and output changes
   */
   static uint32_t lastSend = 0;
   if(millis() - lastSend > 30000)
   {
     lastSend = millis();
     int inputId = 1;
     int inputValue = random(0, 255);
     pubMsg(Sherlocked.sendInput(inputId, inputValue, T_INPUT));
   }

}
