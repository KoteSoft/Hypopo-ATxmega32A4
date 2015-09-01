/*
 * Output.c
 *
 * Created: 20.10.2013 16:00:21
 *  Author: Слава
 */ 

#include "avr/io.h"
#include <util/delay.h>
#include "GlobalConstants.h"
#include "Output.h"

volatile uint16_t soundTimer = 0;

void Sound_On();
void Sound_Off();

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
	Sound_TimeOn(50);
	_delay_ms(50);
	Sound_TimeOn(50);
}

void Sound_TimeOn(uint16_t time_ms)
{
	soundTimer = time_ms / H_Step;
}

void Sound_Loop()
{
	if (soundTimer > 0)
	{
		Sound_On();
		soundTimer--;
	} 
	else
	{
		Sound_Off();
	}
}