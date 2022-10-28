#ifndef IO_CONFIG_H
#define IO_CONFIG_H

// Firmware version
char firmware_version[] = "0.7";                                // inputs with callbacks and debouncing!

// SETTINGS

// mqtt/ace settings
char hostname[] ="controller";                                  // the hostname for board  <= REPLACE
const char gen_topic[] = "alch";  
const char puzzle_topic[] = "alch/centrepiece"; 
const char module_topic[] = "alch/centrepiece/controller";      // the module name of the board <= REPLACE


// controller settings
//pwm 
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 12;

// motor controllers
const int motor_controller_on_off_delay = 5000;  // delay between the HIGH and LOW for the motor controllers in microseconds

// input debounce time
const int debounce_time = 10;


#endif    // IO_CONFIG_H