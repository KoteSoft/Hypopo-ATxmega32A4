/*
 * Filters.c
 *
 * Created: 02.11.2013 18:31:14
 *  Author: Слава
 */ 
#include "Filters.h"

float LowPassFilter(float x, float xp, float dT, float RC)
{
	float A = dT / (RC + dT);	
	return x * A + xp * (1.0 - A);	
}

float ModifyLowPassFilter(float x, float xp, float dT, float RC, float k)
{
	float A = dT / (RC + dT);
	return k * (x * A + xp * (1.0 - A));
}

float ComplementaryFilter(float x1, float x2, float k)
{
	return x1 * k + x2 * (1.0 - k);
}

float Butterworth2Filter(float xi[3], float yi[2], struct butterwrthParams_t params)
{	
	return params.a0 * xi[0] + params.a1 * xi[1] + params.a2 * xi[2] - params.b1 * yi[0] - params.b2 * yi[1];	
}