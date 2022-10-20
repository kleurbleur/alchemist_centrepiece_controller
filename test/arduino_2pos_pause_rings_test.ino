/*
Nanotec N5-2 controller sync test
*/

#include <Arduino.h>

const int n1_pin1 = 2;
const int n1_pin2 = 4;
const int n1_pin3 = 6;
const int n1_pin4 = 8;
// const int n2_pin1 = A0;
// const int n2_pin2 = A2;
// const int n2_pin3 = A4;
// const int n2_pin4 = A5;
const int r_pin1 = 9;
const int r_pin2 = 10;
const int r_pin3 = 11;
const int r_pin4 = 12;



String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
int on_off = 20;

void setup() {
  pincheck("SETUP");
  Serial.begin(9600);

  pinMode(n1_pin1, OUTPUT);
  pinMode(n1_pin2, OUTPUT);
  pinMode(n1_pin3, OUTPUT);
  pinMode(n1_pin4, OUTPUT);
//   pinMode(n2_pin1, OUTPUT);
//   pinMode(n2_pin2, OUTPUT);
//   pinMode(n2_pin3, OUTPUT);
//   pinMode(n2_pin4, OUTPUT);
  pinMode(r_pin1, OUTPUT);
  pinMode(r_pin2, OUTPUT);
  pinMode(r_pin3, OUTPUT);
  pinMode(r_pin4, OUTPUT);
    
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
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

void pincheck(String pin){
    Serial.print(pin);
    Serial.print(": ");
    Serial.print(digitalRead(n1_pin1));
    Serial.print(digitalRead(n1_pin2));
    Serial.print(digitalRead(n1_pin3));
    Serial.print(digitalRead(n1_pin4));
    // Serial.print(digitalRead(n2_pin1));
    // Serial.print(digitalRead(n2_pin2));
    // Serial.print(digitalRead(n2_pin3)); 
    // Serial.print(digitalRead(n2_pin4)); 
    Serial.print(digitalRead(r_pin1));
    Serial.print(digitalRead(r_pin2));
    Serial.print(digitalRead(r_pin3)); 
    Serial.println(digitalRead(r_pin4));     
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

    
    // check for input and set the outputs accordingly  
    else if (inChar == '1'){
      digitalWrite(n2_pin1, !digitalRead(n2_pin1));
      digitalWrite(n1_pin1, !digitalRead(n1_pin1));
      pincheck("1B+1A");
      // delay(on_off);
      // digitalWrite(n2_pin1, !digitalRead(n2_pin1));
      // pincheck("1B");
    }
    else if (inChar == '2'){
      digitalWrite(n2_pin2, !digitalRead(n2_pin2));
      digitalWrite(n1_pin2, !digitalRead(n1_pin2));      
      pincheck("2B+2A");
      delay(on_off);
      digitalWrite(n2_pin2, !digitalRead(n2_pin2));
      digitalWrite(n1_pin2, !digitalRead(n1_pin2));      
      pincheck("2B+2A");
    }  
    else if (inChar == '3'){
      digitalWrite(n2_pin3, !digitalRead(n2_pin3));
      digitalWrite(n1_pin3, !digitalRead(n1_pin3));      
      pincheck("3B+3A");
      delay(on_off);
      digitalWrite(n2_pin3, !digitalRead(n2_pin3));
      digitalWrite(n1_pin3, !digitalRead(n1_pin3));      
      pincheck("3B+3A");
   }
    else if (inChar == '4'){
      digitalWrite(n2_pin4, !digitalRead(n2_pin4));
      pincheck("4B");
      digitalWrite(n2_pin4, !digitalRead(n2_pin4));
      pincheck("4B");
   }

    else if (inChar == '7'){
      digitalWrite(r_pin1, !digitalRead(r_pin1));
      pincheck("r_pin1");
      // delay(on_off);
      // digitalWrite(n2_pin1, !digitalRead(n2_pin1));
      // pincheck("1B");
    }
    else if (inChar == '8'){
      digitalWrite(r_pin2, !digitalRead(r_pin2));
      pincheck("r_pin2");
      delay(on_off);
      digitalWrite(r_pin2, !digitalRead(r_pin2));      
      pincheck("r_pin2");
    }  
    else if (inChar == '9'){
      digitalWrite(r_pin3, !digitalRead(r_pin3));
      pincheck("r_pin3");
      delay(on_off);
      digitalWrite(r_pin3, !digitalRead(r_pin3));
      pincheck("r_pin3");
    }
    else if (inChar == '0'){
      digitalWrite(r_pin4, !digitalRead(r_pin4));
      pincheck("r_pin4");
      delay(on_off);
      digitalWrite(r_pin4, !digitalRead(r_pin4));
      pincheck("r_pin4");
    }
  }
}