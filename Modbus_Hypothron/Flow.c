/*
 * Flow.c
 *
 * Created: 20.10.2013 19:02:51
 *  Author: Слава
 */ 
#include "Flow.h"
#include "Params.h"
#include "Filters.h"
#include <math.h>
#include "GlobalConstants.h"

float Qprev1, Qprev2; //значения ОРВ в предыдущем измерении 
int8_t breathDirection, breathDirectionPre; //вдох/выдох
float flowIntSum; //интегральная сумма потока (объем)
uint16_t breathTimer; //период вдох-выдох
int8_t breathDirectionStack[3];

//Кусочно-линейная интерполяция
float PLI(float X0, float Y0, float X1, float Y1, float X);
float PLI(float X0, float Y0, float X1, float Y1, float X)
{
	return Y0 + (Y1 - Y0) * ((X - X0) / (X1 - X0));
}

float OutCalc(float p)
{
	float temp;
	
	if (p < 0.0)
	{
		//temp = -savedParameters[K_CE].value * savedParameters[CE].value * sqrt(2 * Rho * (-p) * savedParameters[CE].value);
		temp = -1.;
	} 
	else
		//temp = savedParameters[K_CE].value * savedParameters[CE].value * sqrt(2 * Rho * p * savedParameters[CE].value);
		temp = 0.294967 * exp(1.29301 * p) / 60.0;// *0.8695652;
	return temp;
	//return ModifyLowPassFilter(temp, Qprev2, savedParameters[DT_F2].value, savedParameters[RC_F2].value, savedParameters[K_F2].value);	
}

void FlowCalc()
{
	Measurements[Flow2].value = OutCalc(Measurements[ADC0].value * flow_divider - U0);
	
	if (fabs(Measurements[Flow2].value) > savedParameters[SWBR_F2].value)
	{
		Measurements[FlowT].value = Measurements[Flow2].value;
	} 
	else
	{
		Measurements[FlowT].value = Measurements[Flow1].value;
	}
}

