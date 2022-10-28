#ifndef SHERLOCKED_FUNC_H
#define SHERLOCKED_FUNC_H

#include <Arduino.h>
#include <Sherlocked.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <ETH.h>
#include <config.h>

WiFiClient ethclient;
PubSubClient client(ethclient);


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


#endif    // SHERLOCKED_FUNC_H