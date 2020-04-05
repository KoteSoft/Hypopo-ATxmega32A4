/*
 * MAX30102.c
 *
 * Created: 04.03.2017 22:58:32
 *  Author: Slava
 */ 
#include "MAX30102.h"
#include <avr/io.h>
#include <util/delay.h>

void MAX30102_Init()
{
	//TWIE_MASTER_CTRLA = 1 << TWI_MASTER_ENABLE_bp;
	//TWIE_MASTER_BAUD = (uint8_t)(F_CPU / (2 * (5 + 1000)));
	
	//TWIE_MASTER_ADDR = 0xAF;
	//while ()
	
	TWIE_MASTER_BAUD = (uint8_t)(F_CPU / (2 * (5 + 1000)));//0x9B;    //TWI Frequency: 100kHz
	TWIE_MASTER_CTRLA = TWI_MASTER_ENABLE_bm | TWI_MASTER_WIEN_bp;
	TWIE_MASTER_STATUS |= TWI_MASTER_BUSSTATE1_bp;

	TWIE_MASTER_ADDR = 0xA6;//Send Slave Add Write Bit
	while(!(TWIE_MASTER_STATUS & TWI_MASTER_WIF_bm)); //Wait for Write Int Flag High
	
	TWIE_MASTER_DATA = 0x48;//Send Command Byte for Pressure Data
	while(!(TWIE_MASTER_STATUS & TWI_MASTER_WIF_bm));
	
	TWIE_MASTER_CTRLC = 0x03;//Stop
	_delay_ms(10);	//Required Delay	
}