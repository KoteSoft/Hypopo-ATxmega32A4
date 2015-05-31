/*
 * Output.c
 *
 * Created: 20.10.2013 16:00:21
 *  Author: Слава
 */ 

#include "avr/io.h"
#include <util/delay.h>

void Sound_On()
{
	TCC0.CCB = 0x4000;
}

void Sound_Off()
{	
	TCC0.CCB = 0x0000;
}

void Sound_StartMusic()
{
	TCC0.CCB = 0x4000;
	_delay_ms(50);
	TCC0.CCB = 0x0000;
	_delay_ms(50);
	TCC0.CCB = 0x4000;
	_delay_ms(50);
	TCC0.CCB = 0x0000;
	_delay_ms(50);
}