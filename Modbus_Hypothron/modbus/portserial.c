/*
 * FreeModbus Libary: ATMega168 Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *   - Initial version and ATmega168 support
 * Modfications Copyright (C) 2006 Tran Minh Hoang:
 *   - ATmega8, ATmega16, ATmega32 support
 *   - RS485 support for DS75176
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.6 2006/09/17 16:45:53 wolti Exp $
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "..\main.h"
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"



//#define UART_UCSRB  UCSR0B

void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
#ifdef RTS_ENABLE
    //UCSRB |= _BV( TXEN ) | _BV(TXCIE);
	MODBUS_UART.CTRLB |= USART_TXEN_bp;
	MODBUS_UART.CTRLA |= USART_TXCINTLVL_HI_gc;
#else
    //UCSRB |= _BV( TXEN );
	MODBUS_UART.CTRLB |= USART_TXEN_bm;
#endif

    if( xRxEnable )
    {
        //UCSRB |= _BV( RXEN ) | _BV( RXCIE );
		MODBUS_UART.CTRLB |= USART_RXEN_bm;
		MODBUS_UART.CTRLA |= USART_RXCINTLVL_HI_gc;
    }
    else
    {
        //UCSRB &= ~( _BV( RXEN ) | _BV( RXCIE ) );
		MODBUS_UART.CTRLB &= ~(USART_RXEN_bp); 
		MODBUS_UART.CTRLA &= ~(USART_RXCINTLVL_HI_gc);
    }

    if( xTxEnable )
    {
        //UCSRB |= _BV( TXEN ) | _BV( UDRE );
		MODBUS_UART.CTRLB |= USART_TXEN_bp;
		MODBUS_UART.CTRLA |= USART_DREINTLVL_HI_gc;
#ifdef RTS_ENABLE
        RTS_HIGH;
#endif
    }
    else
    {
        MODBUS_UART.CTRLA &= ~(USART_DREINTLVL_HI_gc);
    }
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    /* prevent compiler warning. */
    (void)ucPORT;
	
    //UBRR = UART_BAUD_CALC( ulBaudRate, F_CPU );
	MODBUS_UART.BAUDCTRLA = 109;
	MODBUS_UART.BAUDCTRLB = -4 << USART_BSCALE_gp;

    switch ( eParity )
    {
        case MB_PAR_EVEN:			
            MODBUS_UART.CTRLC = USART_PMODE_EVEN_gc;
            break;
        case MB_PAR_ODD:
            MODBUS_UART.CTRLC = USART_PMODE_ODD_gc;
            break;
        case MB_PAR_NONE:
			MODBUS_UART.CTRLC = USART_PMODE_DISABLED_gc;
            break;
    }

    switch ( ucDataBits )
    {
        case 8:
            MODBUS_UART.CTRLC |= USART_CHSIZE_8BIT_gc;
            break;
        case 7:
            MODBUS_UART.CTRLC |= USART_CHSIZE_7BIT_gc;
            break;
    }

    vMBPortSerialEnable( FALSE, FALSE );

#ifdef RTS_ENABLE
    RTS_INIT;
#endif
    return TRUE;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    MODBUS_UART.DATA = ucByte;
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    *pucByte = MODBUS_UART.DATA;
    return TRUE;
}

ISR( USARTC1_DRE_vect, ISR_BLOCK)
{
    pxMBFrameCBTransmitterEmpty(  );
}

ISR( USARTC1_RXC_vect, ISR_BLOCK)
{
	pxMBFrameCBByteReceived(  );
}

#ifdef RTS_ENABLE
ISR(USARTC1_TXC_vect)
{
    RTS_LOW;
}
#endif

