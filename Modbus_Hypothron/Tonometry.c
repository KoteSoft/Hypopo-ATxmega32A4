/*
 * Tonometry.c
 *
 * Created: 21.02.2016 22:59:24
 *  Author: Слава
 */ 

#include "Tonometry.h"
#include <stdint.h>
#include <avr/io.h>
#include "Params.h"

void nibpADCConv();

volatile static uint32_t overflowCount;
volatile static uint8_t tonometryState;

void tonometryOpenValve()
{
	PORTE.OUT &= ~(1 << 0);
}

void tonometryCloseValve()
{
	PORTE.OUT |= 1 << 0;
}

void tonometryStartCompressor()
{
	PORTE.OUT |= 1 << 1;
}

void tonometryStopCompressor()
{
	PORTE.OUT &= ~(1 << 1);
}

void setDelay(uint32_t time_ms)
{
	overflowCount = time_ms / 10;
}

uint8_t nibpTonesCheck()
{
	return 1;
}

void nibpStartMeas()
{
	tonometryState = 1;
	nibpStateMachine();
}

void nibpStateMachine()
{ 
	nibpADCConv();
	switch (tonometryState)
	{
	case 1:
		//Начало измерения, закрытие выпускного клапана
		tonometryCloseValve();
		setDelay(800);
		tonometryState = 2;
		break;
		
	case 2:
		//Включение компрессора
		tonometryStartCompressor();
		setDelay(3000);
		tonometryState = 3;
		break;
		
	case 3:
		if (Measurements[PRESS_TUBE].value > savedParameters[PRESS_L1].value)
		{
			tonometryStopCompressor();
			tonometryState = 4;
		}
		else
		{
			setDelay(3000);
			tonometryState = 3;
		}
		break;
		
	case 4:
		if (nibpTonesCheck())
		{
			Measurements[NIBP_S].value = Measurements[PRESS_TUBE].value;
			tonometryState = 5;
		}
		else
			tonometryState = 4;
		
		if (Measurements[PRESS_TUBE].value < savedParameters[PRESS_MIN].value)
		{
			tonometryState = 6;
		}
		break;	
		
	case 5:
		if (!nibpTonesCheck())
		{
			Measurements[NIBP_D].value = Measurements[PRESS_TUBE].value;
			tonometryState = 6;
		}
		else
			tonometryState = 5;
			
		if (Measurements[PRESS_TUBE].value < savedParameters[PRESS_MIN].value)
		{
			tonometryState = 6;
		}
		break;
		
	case 6:
		tonometryOpenValve();
		tonometryState = 0;
		break;
		
	default: tonometryState = 0;	
	}
}

void nibpADCConv()
{
	Measurements[PRESS_TUBE].value = savedParameters[PRESS_EXP1].value + Measurements[ADC7].value * savedParameters[PRESS_EXP2].value + Measurements[ADC7].value * Measurements[ADC7].value * savedParameters[PRESS_EXP3].value;
}

void nibpTimerLoop()
{
	if (overflowCount == 0)
	{
		nibpStateMachine();
	}
	else
	{
		overflowCount--;
	}
}