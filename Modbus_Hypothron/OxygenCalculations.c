/*
 * OxygenCalculations.c
 *
 * Created: 30.10.2013 20:33:49
 *  Author: Слава
 */ 
#include "OxygenCalculations.h"
#include "ADC.h"
#include "Params.h"
#include "GlobalConstants.h"
#include "main.h"

float O2_pre;

uint8_t O2CoeffCalc()
{
	nonsavedParameters[O2_K].value = O2_in_air / Measurements[ADC4].value;
	usRegHoldingBuf[MB_O2_K] = nonsavedParameters[O2_K].array[0];
	usRegHoldingBuf[MB_O2_K] = nonsavedParameters[O2_K].array[1];
	
	if (nonsavedParameters[O2_K].value > nonsavedParameters[O2_K_MIN].value && nonsavedParameters[O2_K].value < nonsavedParameters[O2_K_MAX].value)
	{
		return 5;
	}
	
	return 1;
}