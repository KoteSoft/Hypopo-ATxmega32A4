/*
 * Filters.h
 *
 * Created: 02.11.2013 18:31:00
 *  Author: Слава
 */ 


#ifndef FILTERS_H_
#define FILTERS_H_

struct butterwrthParams_t 
{
	float a0, a1, a2, b1, b2;
};

float LowPassFilter(float x, float xp, float dT, float RC);
float ModifyLowPassFilter(float x, float xp, float dT, float RC, float k);
float ComplementaryFilter(float x1, float x2, float k);
float Butterworth2Filter(float xi[3], float yi[2], struct butterwrthParams_t params);

#endif /* FILTERS_H_ */