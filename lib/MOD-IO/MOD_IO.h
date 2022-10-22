/*
 * MOD_IO.h
 * 
 * Copyright 2013 OLIMEX LTD/Stefan Mavrodiev <support@olimex.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 */

#ifndef MOD_IO_H
#define MOD_IO_H

#include <inttypes.h>

#define SET_RELAY		0x10
#define GET_PORT		0x20
#define GET_AN1		0x30
#define GET_AN2		0x31
#define GET_AN3		0x32
#define GET_AN4		0x33
#define SET_ADDRESS 	0xF0
 
#define	ON	1
#define	OFF	0
	
#define	RELAY1 0x01
#define	RELAY2 0x02
#define	RELAY3 0x04
#define	RELAY4 0x08

class MOD_IO
{
	private:
	 uint8_t address;
	 uint8_t relay_status;
	public:
		MOD_IO();
		MOD_IO(uint8_t);
		void begin();
		void setRelay(uint8_t relay, uint8_t state);
		uint8_t digitalRead(uint8_t pin);
		uint8_t digitalReadAll();
		uint16_t analogRead(uint8_t pin);
		void setAddress(uint8_t newAddress);	
};


#endif