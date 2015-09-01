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
	Measurements[O2_K].value = O2_in_air / Measurements[ADC1].value;

	if (Measurements[O2_K].value > savedParameters[O2_K_MIN].value && Measurements[O2_K].value < savedParameters[O2_K_MAX].value)
	{
		return 5;
	}
	
	return 1;
}