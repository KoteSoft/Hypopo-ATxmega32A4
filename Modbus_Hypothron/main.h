/*
 * main.h
 *
 * Created: 10.09.2013 17:58:46
 *  Author: Слава
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "modbus/mb.h"
#include "modbus/mbport.h"
#include "Params.h"

#define REG_INPUT_START 101	//100
#define REG_INPUT_NREGS (measurements_list_SIZE + 1) * 2

#define REG_HOLDING_START 1201	//1200
#define REG_HOLDING_NREGS 340

#define REG_COILS_START     10
#define REG_COILS_SIZE      16

#define MODBUS_UART USARTC1
#define UART_BAUD_RATE          25600
#define UART_BAUD_CALC(UART_BAUD_RATE,F_OSC) \
( ( F_OSC ) / ( ( UART_BAUD_RATE ) * 16UL ) - 1 )

static USHORT   usRegInputStart = REG_INPUT_START;
extern USHORT   usRegInputBuf[REG_INPUT_NREGS];
static USHORT   usRegHoldingStart = REG_HOLDING_START;
extern USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];

extern unsigned char ucRegCoilsBuf[REG_COILS_SIZE / 8];


#endif /* MAIN_H_ */