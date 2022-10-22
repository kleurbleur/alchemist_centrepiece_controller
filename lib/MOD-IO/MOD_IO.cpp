/*
 * MOD_IO.cpp
 * 
 * Copyright 2013 OLIMEX LTD/Stefan Mavrodiev <support@olimex.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 *	MOD-IO Additional Info by Ken Segler.txt
 *
 * Description:
 * This library requires an I2C slave device to interface the following peripherals 
 * on the board: 
 *  - Digital outputs O1, O2, O3, O4
 *  - Digital inputs IN1, IN2, IN3, IN4
 *  - Analogue inputs AIN1, AIN2, AIN3, AIN4
 *
 * There is actually nothing specific about the I2C protocol. Default address of 
 * the slave is 0x58. When addressed, the device acknowledges reception with an ACK 
 * flag set to 0 to indicate its presence. 
 *
 */
 

#include "MOD_IO.h"
#include <Wire.h>
#include <Arduino.h>


/*
 * Class variables
 */

/*
 * Constructors
 */
MOD_IO::MOD_IO(){
}

MOD_IO::MOD_IO(uint8_t addr)
{
	address = addr;
}

void MOD_IO::begin()
{
	// Initialize the Wire Library
	Wire.begin();
	relay_status = 0x00;
}

/***********************************************************************************
	1) Set states of the digital outputs on the board. The board features four relay 
	outputs that can be set together with one command. The command should have the 
	following 3 byte format:

	************************************ 
	S aaaaaaaW cccccccc 0000dddd P 
	************************************

	Where S - start condition 
	aaaaaaa - slave address of the board W - write mode, should be 0 
	cccccccc - command code, should be 0x10 
	dddd - bitmap of the output states, i.e. bit0 corresponds to REL1, bi1 to REL2 
	and so on. '1' switches the relay ON, '0' switches to OFF state. 
	P - Stop condition
***********************************************************************************/
void MOD_IO::setRelay(uint8_t relay, uint8_t state)
{
	if(state == 1)
		relay_status |= relay;
	else if(state == 0)
		relay_status &= ~relay;
	Wire.beginTransmission(address);
	Wire.write(SET_RELAY);
	Wire.write(relay_status);
	Wire.endTransmission();
}

/***********************************************************************************
	2) Get states of the digital inputs of the board. The board features four 
	optoisolated inputs read together with one command. The command should have the 
	following format:

	************************************ 
	S aaaaaaaW cccccccc P S aaaaaaaR 0000dddd P 
	************************************

	Where S - start condition 
	aaaaaaa - slave address of the board W - write mode, should be 0 
	cccccccc - command code, should be 0x20 
	P - Stop condition 
	R - read mode, should be 1 
	dddd - bitmap of the input states received from the MOD-IO board, i.e. bit0 
	corresponds to IN1, bit1 to IN2 and so on. '1' means that power is applied to 
	the optocoupler, '0' means the opposite.
***********************************************************************************/
uint8_t MOD_IO::digitalRead(uint8_t pin)
{
	uint8_t data;

	Wire.beginTransmission(address);
	Wire.write(GET_PORT);
	Wire.endTransmission();
	Wire.requestFrom((int)address, 1);
	data = Wire.read();
	return !!(data & pin);
}

uint8_t MOD_IO::digitalReadAll(void)
{
	uint8_t data;

	Wire.beginTransmission(address);
	Wire.write(GET_PORT);
	Wire.endTransmission();
	Wire.requestFrom((int)address, 1);
	data = Wire.read();
	return (data);
}

/***********************************************************************************
	3) Get the voltage applied to one of the analogue inputs of the board. The board 
	features four 10bit resolution analogue inputs (input voltages from 0 - 3.3V) 
	and each of them is read with a separate command. Command should have the 
	following common format:

	************************************ 
	S aaaaaaaW cccccccc P S aaaaaaaR dddddddd 000000dd P 
	************************************

	Where S - start condition 
	aaaaaaa - slave address of the board W - write mode, should be 0 
	cccccccc - command code, should be 0x30 for AIN1, 0x31 for AIN2, 
	0x31 for AIN3, 0x31 for AIN4. 
	P - Stop condition R - read mode, should be 1 
	dddddddd 000000dd ? Little Endian (LSB:MSB) 10bit binary encoded value 
	corresponding to the input voltage. Range is 0 - 0x3FF and voltage on the pin is 
	calculated using the following simple formula: voltage = (3.3 / 1024) * 
	(readvalue) [Volts]
***********************************************************************************/
uint16_t MOD_IO::analogRead(uint8_t pin)
{
	uint16_t data = 0;
	uint8_t a[2];

	Wire.beginTransmission(address);
	if (pin == 1)
		Wire.write(GET_AN1);
	else if (pin == 2)
		Wire.write(GET_AN2);
	else if (pin == 3)
		Wire.write(GET_AN3);
	else if (pin == 4)
		Wire.write(GET_AN4);
	else
		return(0);
	Wire.endTransmission();
	Wire.requestFrom((int)address, 2);
	a[0] = Wire.read();
	a[1] = Wire.read();
  
	data = (a[1] << 8) | a[0];
	return data;
}

/***********************************************************************************
	4) Set new slave address to the board. The board ships with default 7bit address 
	0x01 that can be changed to any other 7bit value in order for the host to 
	interface more than 1 device connected on the bus at the same time. Change is 
	stored in EEPROM and thus is permanent between power cycles. Changing the 
	address requires the following command format:

	************************************ 
	S aaaaaaaW cccccccc 0ddddddd P 
	************************************

	Where S - start condition aaaaaaa - slave address of the board W - write mode, 
	should be 0 cccccccc - command code, should be 0xF0 ddddddd - new 7bit address 
	to update P - Stop condition

	NB!! To protect the device from accidental address updates the user should hold 
	the onboard button pressed (not the RESET button!) while issuing the command. 
	Successful update is indicated with the onboard status LED being contently lit 
	for 2-3 seconds. Address is immediately updated so the board will not respond to 
	its old address any more.

	IMPORTANT: The default address of the board could be restored if the onboard 
	button is held pressed at power up for more than 4 seconds. 
	This situation is indicated by the onboard LED blinking fast for the timeout period. 
	When the fast blinking ends default address is restored.
***********************************************************************************/
void MOD_IO::setAddress(uint8_t newAddress)
{
	Wire.beginTransmission(address);
	Wire.write(SET_ADDRESS);
	Wire.write(newAddress);
	Wire.endTransmission();
}