/*
 * Sherlocked.h - The official Sherlocked library.
 *
 * Copyright (c) 2022
 * Interactive Matter, by Serge Offermans
 * serge@interactivematter.nl
 *
 * All rights reserved. 
 * LAST UPDATE: 12-07-2022
*/

/*! \mainpage The Sherlocked Library
 *	This Arduino Library is used to manage wireless communication with the Sherlocked Rooms. <br /> <br /> 
	<a href="functions_func.html">All available functions are documented here</a> 
 */

#ifndef Sherlocked_h
#define Sherlocked_h

#include "Arduino.h"
#include "ArduinoJson.h"

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define SHERLOCKED_OUTPUT_CALLBACK std::function<void(int meth, int numOutputs, int ids[], int vals[], int triggerID)> outputCallback
#else
#define SHERLOCKED_OUTPUT_CALLBACK void (*callback)(int meth, int numOutputs, int ids[], int vals[], int triggerID)
#endif

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define SHERLOCKED_INPUT_CALLBACK std::function<void(int meth, int numInputs, int ids[], int vals[], int triggerID)> inputCallback
#else
#define SHERLOCKED_INPUT_CALLBACK void (*callback)(int meth, int numInputs, int ids[], int vals[], int triggerID)
#endif

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define SHERLOCKED_COMMAND_CALLBACK std::function<void(int meth, int cmd, const char * value, int triggerID)> commandCallback
#else
#define SHERLOCKED_COMMAND_CALLBACK void (*callback)(int meth, int cmd, const char * value, int triggerID)
#endif

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define SHERLOCKED_STATE_CALLBACK std::function<void(int meth, int state, int triggerID)> stateCallback
#else
#define SHERLOCKED_STATE_CALLBACK void (*callback)(int meth, int state, int triggerID)
#endif

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define SHERLOCKED_JSON_CALLBACK std::function<void(JsonObject& json)> jsonCallback
#else
#define SHERLOCKED_JSON_CALLBACK void (*jsonCallback)(JsonObject& json)
#endif

static const uint16_t MESSAGE_LENGTH = 1024;
static const char SHERLOCKED_VERSION[] = "Sherlocked_v0.7";

#define UNDEFINED -1
#define dbf Serial.printf

enum StateConstants {
  S_UNKNOWN,
  S_IDLE,
  S_ACTIVE,
  S_SOLVED,

  NUM_STATE_CONSTANTS
};

enum MethodConstants {
  M_GET,
  M_PUT,
  M_INFO, 
  
  NUM_METHOD_CONSTANTS
};

enum TriggerConstants {
  T_STARTUP,
  T_RESET,
  T_INPUT,
  T_STATE,
  T_TIME,
  T_LOGIC,
  T_SERVER,
  T_REQUEST,
  T_DISCONNECT,
  T_CONNECTED,
  
  NUM_TRIGGER_CONSTANTS
};

enum CommandConstants {
  CMD_RESET,
  CMD_SYNC,
  CMD_REBOOT,
  CMD_OTA,
  INFO_SYSTEM,
  INFO_STATE,
  INFO_FULLSTATE,
  
  NUM_COMMAND_CONSTANTS
};

class SherlockedClass
{
  public:
  	/*	Constructor and destructor	*/
	SherlockedClass();
	~SherlockedClass();
		
	// void begin( HardwareSerial & port = Serial1, uint32_t baud = 115200 );
	
	// void setSerial(HardwareSerial &_port );
	void setName(const char * n);
	char * getName();

	void setOutputCallback( SHERLOCKED_OUTPUT_CALLBACK );	
	void setInputCallback( SHERLOCKED_INPUT_CALLBACK );	
	void setStateCallback( SHERLOCKED_STATE_CALLBACK );	
	void setCommandCallback( SHERLOCKED_COMMAND_CALLBACK );	
	void setJSONCallback( SHERLOCKED_JSON_CALLBACK );

	char * sendState(int state, int trig);
	char * sendInput(int id, int val, int trig);
	char * sendInputs(int num, int ids[], int vals[], int trig);
	char * sendOutput(int id, int val, int trig);
	char * sendOutputs(int num, int ids[], int vals[], int trig);
	char * send(JsonObject& json);

	bool parse(char * msg);

	const char * getTriggerStr(int id);
	const char * getMethodStr(int id);
	const char * getCommandStr(int id);
	const char * getStateStr(int id);

	int getTriggerID(const char * str);
	int getMethodID(const char * str);
	int getCommandID(const char * str);
	int getStateID(const char * str);
	
	
	const char * methodStrings[NUM_METHOD_CONSTANTS] 		= {"get", "put", "info"};
	const char * triggerStrings[NUM_TRIGGER_CONSTANTS] 	= {"startup", "reset", "input", "state", "time", "logic", "server", "request", "disconnect", "connected"};
	const char * commandStrings[NUM_COMMAND_CONSTANTS] 	= {"reset", "sync", "reboot", "ota", "system", "state", "full"};
	const char * stateStrings[NUM_STATE_CONSTANTS] 			= {"unknown", "idle", "active", "solved"};

  private:  	
  
  // bool inputCallbackDefined = false;
  // bool outputCallbackDefined = false;
  // bool stateCallbackDefined = false;
  // bool commandCallbackDefined = false;
  // bool jsonCallbackDefined = false;

  char _name[64];
  char _pubBuf[MESSAGE_LENGTH];

	SHERLOCKED_INPUT_CALLBACK;
	SHERLOCKED_OUTPUT_CALLBACK;
	SHERLOCKED_COMMAND_CALLBACK;
	SHERLOCKED_STATE_CALLBACK;
	SHERLOCKED_JSON_CALLBACK;
};

extern SherlockedClass Sherlocked;

#endif
