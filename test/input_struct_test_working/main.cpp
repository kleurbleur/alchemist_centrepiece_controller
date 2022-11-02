#include <Arduino.h>

int _inputsPins[9] = {3,4,14,33,36,39};
int _inputsPinsArraySize = sizeof(_inputsPins)/sizeof(int);

enum input_names {
    CONTROLLER_ARM_A_TARGET_TOP,
    CONTROLLER_ARM_A_TARGET_BOTTOM,
    CONTROLLER_ARM_B_TARGET_TOP,
    CONTROLLER_ARM_B_TARGET_BOTTOM,
    CONTROLLER_RINGS_MOVING,
    CONTROLLER_RING_TARGET,
    ARM_B_SENSOR_TOP,
    ARM_B_SENSOR_BOTTOM,
    ARM_A_SENSOR_TOP,
    ARM_A_SENSOR_BOTTOM,
    CONTROLLER_ARMS_MOVING,
    NUM_INPUT
};

struct input {
    uint8_t id;
    uint8_t value;
    uint8_t pin;
    uint32_t debounce;
    uint32_t last;
};


input _input[NUM_INPUT];

void initInputs()
{
    Serial.println("Setup commencing");
    for (int i = 0; i < NUM_INPUT; i++)
    {
        Serial.printf("Setup %i \n", i);
        _input[i].id = i+1;
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

void getInputsState()
{
    for (int i = 0; i < NUM_INPUT; i++)  
    {
        Serial.printf("_input.id: %i _input.val:%i _input.debounce: %i _input.last: %i  _input.pin: %i \n",_input[i].id, _input[i].value, _input[i].debounce, _input[i].last, _input[i].pin);
    }    
}



void setup(){
    Serial.begin(115200);
    
    delay(1500); // platformio monitor setup time 

    Serial.println("Welcome");
    Serial.printf("input_number: %i \n", _inputsPinsArraySize);

    initInputs();
    getInputsState();

    Serial.println("Setup done");


}


void loop() {

    delay(200);

    int one = digitalRead(_input[CONTROLLER_ARM_A_TARGET_TOP].pin);
    int two = digitalRead(_input[CONTROLLER_ARM_A_TARGET_BOTTOM].pin);
    int three = digitalRead(_input[CONTROLLER_ARM_B_TARGET_TOP].pin);
    int four = digitalRead(_input[CONTROLLER_ARM_B_TARGET_BOTTOM].pin);
    int five = digitalRead(_input[CONTROLLER_RINGS_MOVING].pin);
    int six = digitalRead(_input[CONTROLLER_RING_TARGET].pin);


    Serial.printf("%i%i%i%i%i%i\n", one, two, three, four, five, six);
 

    // getInputsState();

}