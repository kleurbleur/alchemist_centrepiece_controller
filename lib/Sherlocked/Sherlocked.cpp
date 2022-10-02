/*
 * Sherlocked.cpp - The official Sherlocked library.
 *
 * Copyright (c) 2022
 * Interactive Matter, by Serge Offermans
 * serge@interactivematter.nl
 *
 * All rights reserved. 
 * LAST UPDATE: 12-07-2022
 *
*/

#include "Sherlocked.h"


/** 
	Default constructor called when importing the library.
**/
SherlockedClass::SherlockedClass()
{

}

SherlockedClass::~SherlockedClass()
{

}

void SherlockedClass::setCommandCallback(SHERLOCKED_COMMAND_CALLBACK) {
	this->commandCallback = commandCallback;
	// this->commandCallbackDefined = true;
}

void SherlockedClass::setOutputCallback(SHERLOCKED_OUTPUT_CALLBACK) {
	this->outputCallback = outputCallback;
	// this->outputCallbackDefined = true;
}

void SherlockedClass::setStateCallback(SHERLOCKED_STATE_CALLBACK) {
	this->stateCallback = stateCallback;
	// this->stateCallbackDefined = true;
}

void SherlockedClass::setInputCallback(SHERLOCKED_INPUT_CALLBACK) {
	this->inputCallback = inputCallback;
	// this->inputCallbackDefined = true;
}

void SherlockedClass::setJSONCallback(SHERLOCKED_JSON_CALLBACK) {
	this->jsonCallback = jsonCallback;
	// this->jsonCallbackDefined = true;
}

void SherlockedClass::setName(const char * n) {
	strcpy(this->_name, n);
}

char * SherlockedClass::getName()
{
	return this->_name;	
}

char * SherlockedClass::send(JsonObject& root)
{
	root["sender"] = getName();
  root["method"] 	= getMethodStr(M_INFO);
  root.printTo(_pubBuf, sizeof(_pubBuf));
	return _pubBuf;
}

char * SherlockedClass::sendState(int state, int trig)
{
	StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sender"] = getName();
  root["connected"] = true;
  if(state != UNDEFINED)
  {
  	root["state"] = getStateStr(state);	
  }
  root["method"] 	= getMethodStr(M_INFO);
  root["trigger"] = getTriggerStr(trig);
  root.printTo(_pubBuf, sizeof(_pubBuf));
	return _pubBuf;
}

char * SherlockedClass::sendInputs(int len, int ids[], int vals[], int trig)
{
	StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sender"] = getName();

  JsonArray& outputs = root.createNestedArray("inputs");
  for (int i = 0; i < len; i++)
  {
    JsonObject& nested = outputs.createNestedObject();
    nested["id"]    = ids[i];
    nested["value"] = vals[i];
  }
  root["method"] 	= getMethodStr(M_INFO);
  root["trigger"] = getTriggerStr(trig);
  root.printTo(_pubBuf, sizeof(_pubBuf));
	return _pubBuf;
}

char * SherlockedClass::sendInput(int id, int val, int trig)
{
  int ids[1]  = {id};
  int vals[1] = {val};
  return sendInputs(1, ids, vals,trig);
}

char * SherlockedClass::sendOutputs(int len, int ids[], int vals[], int trig)
{
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sender"] = getName();

  JsonArray& outputs = root.createNestedArray("outputs");
  for (int i = 0; i < len; i++)
  {
    JsonObject& nested = outputs.createNestedObject();
    nested["id"]    = ids[i];
    nested["value"] = vals[i];
  }
  root["method"] 	= getMethodStr(M_INFO);
  root["trigger"] = getTriggerStr(trig);
  root.printTo(_pubBuf, sizeof(_pubBuf));
  return _pubBuf;
  // return pubMsg(_pubBuf);
}

char * SherlockedClass::sendOutput(int id, int val, int trig)
{
  int ids[1]  = {id};
  int vals[1] = {val};
  return sendOutputs(1, ids, vals,trig);
}

bool SherlockedClass::parse(char * incomingMessage)
{
  // Serial.print("PARSING: ");
  // Serial.println(incomingMessage);

  // possibly make the size of inc mes len
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;

  JsonObject &root = jsonBuffer.parseObject(incomingMessage);

  // Test if parsing succeeds.
  if (!root.success()) {
		Serial.println(("MQTT JSON parseObject() failed"));
		//    Serial.println("{\"err\":\"events\"}");
  }
  else
  {
		if (root.containsKey("sender"))
		{
		  const char* s = root["sender"];
		  if (strcmp(s, this->_name) == 0) // We sent this message, dont parse!
		  {
			return false;
		  }
		}
		else
		{ // discard messages without teh sender parameter defined
		  dbf("No Sender Defined, ignore message\n");
		  return false;
		}

		if (root.containsKey("recipient"))
		{
		  const char* r = root["recipient"];
		  if (strcmp(r, this->_name) != 0) // This message is addressed to someone else!
		  {
			dbf("Recipient defined but not us, ignore message\n");
			return false;
		  }
		}

		if (!root.containsKey("method"))
		{
		  dbf("No method defined, ignore message\n");
		  return false;
		}

		bool handled = false;

		const char* methodStr = root["method"];
		int meth = getMethodID(methodStr);

		const char* sender = root["sender"];

		
		
		dbf("Sherlocked.parse() : ");
		root.printTo(Serial);
		Serial.println();

		if (meth == M_PUT || meth == M_INFO)
		{
		  if (root.containsKey("state"))
		  {
				const char * s = root["state"];
				int stat = getStateID(s);
				if (stat != UNDEFINED)
				{
					if(stateCallback)
					{
						stateCallback(meth, stat, getTriggerID(sender));
					  handled = true;	
					}
				}
		  }

		  if (root.containsKey("inputs"))
		  {
				JsonVariant inputs = root["inputs"];
				if (inputs.is<JsonArray>())
				{
				  int numInputs = inputs.size();
				  // dbf("Got inputs Array with %d inputs\n", numInputs);
				  int ids[numInputs];
				  int vals[numInputs];
				  for (int i = 0; i < numInputs; i++)
				  {
						if (inputs[i].is<JsonObject>())
						{
						  if (inputs[i].as<JsonObject>().containsKey("id") && inputs[i].as<JsonObject>().containsKey("value"))
						  {
							int id  = inputs[i].as<JsonObject>()["id"];
							int val = inputs[i].as<JsonObject>()["value"];
							ids[i] 	= id;
							vals[i] = val;
						  }
						}
				  }
				  if(inputCallback)
				  {
				  	int trig = getTriggerID(sender);
				  	if (trig == UNDEFINED && root.containsKey("trigger"))
				  	{
							trig = getTriggerID(root["trigger"]);
				  	}
						inputCallback(meth, numInputs, ids, vals, trig);
				  	handled = true;
				  }
				}
		  }

		  if (root.containsKey("outputs"))
		  {
				JsonVariant outputs = root["outputs"];
				if (outputs.is<JsonArray>())
				{
				  int numOutputs = outputs.size();
				  // dbf("Got outputs Array with %d outputs\n", numOutputs);
				  int ids[numOutputs];
				  int vals[numOutputs];
				  for (int i = 0; i < numOutputs; i++)
				  {
						if (outputs[i].is<JsonObject>())
						{
						  if (outputs[i].as<JsonObject>().containsKey("id") && outputs[i].as<JsonObject>().containsKey("value"))
						  {
							int id  = outputs[i].as<JsonObject>()["id"];
							int val = outputs[i].as<JsonObject>()["value"];
							ids[i] 	= id;
							vals[i] = val;
						  }
						}
				  }
				  if(outputCallback)
				  {
				  	int trig = getTriggerID(sender);
				  	if (trig == UNDEFINED && root.containsKey("trigger"))
				  	{
							trig = getTriggerID(root["trigger"]);
				  	}
						outputCallback(meth, numOutputs, ids, vals, trig);
				  	handled = true;
				  }
				}
		  }

		  if (root.containsKey("cmd"))
		  {
				const char * c = root["cmd"];
				int cid = getCommandID(c);
				if(cid != UNDEFINED) 
				{
					char val[64];
					strcpy(val, "");
					if (root.containsKey("file"))
					{
						const char* file = root["file"];
						strcpy(val, file);
					}
					if(commandCallback)
					{
						int trig = getTriggerID(sender);
						if (trig == UNDEFINED && root.containsKey("trigger"))
						{
								trig = getTriggerID(root["trigger"]);
						}
						commandCallback(meth, cid, val, trig);
						handled = true;
					}
				}
			}
		}
		else if (meth == M_GET)
		{
			if (root.containsKey("outputs"))
			{
				JsonVariant outputs = root["outputs"];
				if (outputs.is<JsonArray>())
				{
					int numOutputs = outputs.as<JsonArray>().size();
					int ids[numOutputs];
					int vals[numOutputs];
					for (int i = 0; i < numOutputs; i++)
					{
						int id = outputs.as<JsonArray>().get<signed int>(i);
						ids[i] = id;
					}
					if(outputCallback)
					{
						int trig = getTriggerID(sender);
				  	if (trig == UNDEFINED && root.containsKey("trigger"))
				  	{
							trig = getTriggerID(root["trigger"]);
				  	}
						outputCallback(meth, numOutputs, ids, vals, trig);	
						handled = true;
					}
				}
			}

			if (root.containsKey("inputs"))
			{
				JsonVariant inputs = root["inputs"];
				if (inputs.is<JsonArray>())
				{
					int numInputs = inputs.as<JsonArray>().size();
					int ids[numInputs];
					int vals[numInputs];
					for (int i = 0; i < numInputs; i++)
					{
						int id = inputs.as<JsonArray>().get<signed int>(i);
						ids[i] = id;
					}
					if(inputCallback)
					{
						int trig = getTriggerID(sender);
				  	if (trig == UNDEFINED && root.containsKey("trigger"))
				  	{
							trig = getTriggerID(root["trigger"]);
				  	}
						inputCallback(meth, numInputs, ids, vals, trig);
						handled = true;
					}
				}
			}

			if (root.containsKey("info"))
			{
				// const char * c = root["info"];
				// if (strcmp(c, "system") == 0)
				// {
				//   pubInfo(E_REQUEST);
				// }
				// else if (strcmp(c, "state") == 0)
				// {
				//   pubState(E_REQUEST);
				// }
				// else if (strcmp(c, "full") == 0)
				// {
				//   pubFullState(E_REQUEST);
				// }
				const char * c = root["info"];
				int cid = getCommandID(c);
				if(cid != UNDEFINED) 
				{
					char val[64];
					strcpy(val, "");
					if (root.containsKey("file"))
				  	{
						const char* file = root["file"];
						strcpy(val, file);
					}
					if(commandCallback)
					{
						int trig = getTriggerID(sender);
						if (trig == UNDEFINED && root.containsKey("trigger"))
						{
								trig = getTriggerID(root["trigger"]);
						}
						commandCallback(meth, cid, val, trig);
						handled = true;
					}
				}
			}
		}

		// now check if one of the callbacks took the message
		if(!handled) 
		{
			if(jsonCallback)
			{
				jsonCallback(root); 	
			}
			else
			{
		 		dbf("\t ERROR: Incoming message was not handled...");
			}
		}
	}
}


const char * SherlockedClass::getTriggerStr(int id)
{
	if(id >= 0 && id < NUM_TRIGGER_CONSTANTS) {
		return triggerStrings[id];	
	}
	return "undefined";
}

const char * SherlockedClass::getCommandStr(int id)
{
	if(id >= 0 && id < NUM_COMMAND_CONSTANTS) {
		return commandStrings[id];	
	}
	return "undefined";
}

const char * SherlockedClass::getMethodStr(int id)
{
	if(id >= 0 && id < NUM_METHOD_CONSTANTS) {
		return methodStrings[id];	
	}
	return "undefined";
}

const char * SherlockedClass::getStateStr(int id)
{
	if(id >= 0 && id < NUM_STATE_CONSTANTS) {
		return stateStrings[id];	
	}
	return "undefined";
}

int SherlockedClass::getTriggerID(const char * str) 
{
	for (int i = 0; i < NUM_TRIGGER_CONSTANTS; i++)
	{
	   if(strcmp(str, triggerStrings[i]) == 0)
	   {
	     return i;
	   }
	}
 	return UNDEFINED;
}

int SherlockedClass::getCommandID(const char * str) 
{
	for (int i = 0; i < NUM_COMMAND_CONSTANTS; i++)
	{
	   if(strcmp(str, commandStrings[i]) == 0)
	   {
	     return i;
	   }
	}
 	return UNDEFINED;
}


int SherlockedClass::getMethodID(const char * str) 
{
	for (int i = 0; i < NUM_METHOD_CONSTANTS; i++)
	{
	   if(strcmp(str, methodStrings[i]) == 0)
	   {
	     return i;
	   }
	}
 	return UNDEFINED;
}

int SherlockedClass::getStateID(const char * str) 
{
	for (int i = 0; i < NUM_STATE_CONSTANTS; i++)
	{
	   if(strcmp(str, stateStrings[i]) == 0)
	   {
	     return i;
	   }
	}
 	return UNDEFINED;
}


/* Define an instance of the Sherlockedclass here, named Sherlocked */
SherlockedClass Sherlocked;