#include <Arduino.h>

const int inc_length = 3;
const int test_length = 4;
int incoming_array[inc_length] = {5,20,18};
int test_id_array[test_length] = {17,18,19,20};
int test_val_array[test_length] = {1,0,1,34};

int findPositionID(int inc_val, int inc_id_array[], int id_array_length, int id_array[])
{
    int pos = -1;
    bool found_val = false;

    int wanted_val = inc_id_array[inc_val];
    // Serial.print("wanted_val: ");
    // Serial.println(wanted_val);

    for (int i = 0; i < id_array_length; i++)
    {
      if (wanted_val == id_array[i])
      {
        found_val = true;
        // Serial.print("found test_val: ");
        // Serial.print(wanted_val);
        // Serial.print(" at position: ");
        // Serial.println(i);
        pos = i;
      }
      else if (!found_val){
        // Serial.print("test_val not in array: ");
        // Serial.println(wanted_val);
        pos = -1;
      }
    }    
    // Serial.print("pos: ");
    // Serial.println(pos);   
    return pos; 
}


void setup() {
    Serial.begin(115200);

  
}

void loop() {

  delay(2000);
  for (int k = 0; k < inc_length; k++)
  {
    int pos = findPositionID(k, incoming_array, test_length, test_id_array);
    Serial.println(pos);
    if (pos >= 0) {
      Serial.print("ID: ");
      Serial.print(test_id_array[pos]);
      Serial.print(" - Value: ");
      Serial.println(test_val_array[pos]);
    }
    else if (pos == -1)
    {
      Serial.println("input not found");
    }
    

  }

  delay(1000 * 60);

}