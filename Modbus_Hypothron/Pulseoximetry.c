/*
 * Pulseoximetry.c
 *
 * Created: 11.04.2015 17:15:14
 *  Author: Слава
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include "GlobalConstants.h"
#include "Params.h"
#include "Filters.h"
#include <stddef.h>

#define POINT_COUNT 20
#define FILTER_POINT_COUNT 10
#define LOOP_PRESCALLER
#define ADC_R_AC ADC6
#define ADC_IR_AC ADC4
#define ADC_R_DC ADC5
#define ADC_IR_DC ADC3

float arrayIRAC[POINT_COUNT];
float arrayIRDC[POINT_COUNT];
float arrayRAC[POINT_COUNT];
float arrayRDC[POINT_COUNT];
float arrayFilter[FILTER_POINT_COUNT];

struct butterwrthParams_t pulseoxButterworthSettings;

void PulseoximetryInit()
{
	//DAC
	/*
	DACB.CTRLA = DAC_CH0EN_bm | DAC_CH1EN_bm | DAC_ENABLE_bm;
	DACB.CTRLB = DAC_CHSEL_DUAL_gc;
	DACB.CTRLC = DAC_REFSEL_AVCC_gc;
	DACB.TIMCTRL = DAC_CONINTVAL_4CLK_gc | DAC_REFRESH_16CLK_gc;
	DACB.CH0DATA = 0x0000;
	DACB.CH1DATA = 0x0000;
	*/
	PORTB.DIR = 1 << 2 | 1 << 3;
	PORTB.OUT = 1 << 2 | 1 << 3;
	
	//TIMER1 RIR
	TCC1.CTRLA = TC_CLKSEL_DIV1_gc;
	TCC1.CTRLB = TC1_CCBEN_bm | TC_WGMODE_SS_gc;
	TCC1.PER = 0x4FFF;
	TCC1.CCB = 0x2FFF;
	
	//Таймер опроса АЦП пульсоксиметра - 100Гц
	TCD0.CTRLA = TC_CLKSEL_DIV256_gc;
	TCD0.CTRLB = TC_WGMODE_NORMAL_gc;
	TCD0.PER = 1250;//1250;
	TCD0.INTCTRLA = TC_OVFINTLVL_HI_gc;
	
	//Рассчет коэффициентов фильтра Баттерворта ФПГ
	float w = tan((M_PI * Pulseox_fc) / Pulseox_fs);
	float c = 1.0 + 2 * cos(M_PI / 4.0) * w + w * w;
	pulseoxButterworthSettings.b2 = (1.0 - 2.0 * cos(M_PI / 4.0) * w + w * w) / c;
	pulseoxButterworthSettings.b1 = (2.0 * (w * w - 1)) / c;
	pulseoxButterworthSettings.a2 = (w * w) / c;
	pulseoxButterworthSettings.a0 = pulseoxButterworthSettings.a2;
	pulseoxButterworthSettings.a1 = 2 * pulseoxButterworthSettings.a0;
}

float Iaciravg;
uint8_t t = 0;
uint16_t loopCounter = 0;
uint8_t hrState = 0;

void PulseoximetryLoop()
{
	//Сдвигаем четыре буффера пульсоксиметрии на 1
	for(uint8_t i = 0; i < POINT_COUNT - 1; i++)
	{
		arrayIRAC[i] = arrayIRAC[i + 1];
		arrayIRDC[i] = arrayIRDC[i + 1];
		arrayRAC[i] = arrayRAC[i + 1];
		arrayRDC[i] = arrayRDC[i + 1];
	}
	
	for (uint8_t i = 0; i < FILTER_POINT_COUNT - 1; i++)
	{
		arrayFilter[i] = arrayFilter[i + 1];
	}
	
	//Заносим в буфферы последние значения
	arrayIRAC[POINT_COUNT - 1] = Measurements[ADC_IR_AC].value;
	arrayIRDC[POINT_COUNT - 1] = Measurements[ADC_IR_DC].value;
	arrayRAC[POINT_COUNT - 1] = Measurements[ADC_R_AC].value;
	arrayRDC[POINT_COUNT - 1] = Measurements[ADC_R_DC].value;	
	
	float tmpX[3], tmpY[2];
	
	for (uint8_t i = 0; i < 3; i++)
	{
		tmpX[i] = arrayIRAC[POINT_COUNT - 1 - i];
	}
	
	for (uint8_t i = 0; i < 2; i++)
	{
		tmpY[i] = arrayFilter[FILTER_POINT_COUNT - 2 - i];
	}
	
	arrayFilter[FILTER_POINT_COUNT - 1] = LowPassFilter(Butterworth2Filter(tmpX, tmpY,pulseoxButterworthSettings), arrayFilter[FILTER_POINT_COUNT - 2], Pulseox_fs, Pulseox_RC);
	
	if (arrayFilter[FILTER_POINT_COUNT - 1] < arrayFilter[FILTER_POINT_COUNT - 2] && arrayFilter[FILTER_POINT_COUNT - 2] > arrayFilter[FILTER_POINT_COUNT - 3])
	{		
		if (HR_Min < (float)((Pulseox_fs / loopCounter) * 60.0) && HR_Max > (float)((Pulseox_fs / loopCounter) * 60.0))
		{
			Measurements[HR].value = (float)((Pulseox_fs / loopCounter) * 60.0);
		}
		loopCounter = 0;
	}
	
	if (arrayFilter[FILTER_POINT_COUNT - 1] < arrayFilter[FILTER_POINT_COUNT - 2])
	{
		Measurements[HR_AVG].value = 0.5;
	} 
	else
	{
		Measurements[HR_AVG].value = 0.0;
	}
	
	loopCounter++;
}

void PulseoximetryHugeCalculation()
{
	float Iacir, Iacr, Idcir, Idcr, Idcrdev, Idcirdev, Idcravg, Idciravg, R;
	float tempMin, tempMax, tempSumm;
	float tempArrayIRAC[POINT_COUNT];
	float tempArrayIRDC[POINT_COUNT];
	float tempArrayRAC[POINT_COUNT];
	float tempArrayRDC[POINT_COUNT];
	                                                                                                                           
	for(uint8_t i = 0; i < POINT_COUNT; i++)
	{
		tempArrayIRAC[i] = arrayIRAC[i];
		tempArrayIRDC[i] = arrayIRDC[i];
		tempArrayRAC[i] = arrayRAC[i];
		tempArrayRDC[i] = arrayRDC[i];
	}
	
	tempMin = 10.0;
	tempMax = 0.0;
	for (uint8_t i = 0; i < POINT_COUNT; i++)
	{
		if (tempArrayIRAC[i] < tempMin)
			tempMin = tempArrayIRAC[i];
		if (tempArrayIRAC[i] > tempMax)
			tempMax = tempArrayIRAC[i];
	}
	Iacir = tempMax - tempMin;
	Iaciravg = (tempMin + tempMax) / 2.0 - 0.3 * Iacir;
	
	tempMin = 10.0;
	tempMax = 0.0;
	for (uint8_t i = 0; i < POINT_COUNT; i++)
	{
		if (tempArrayRAC[i] < tempMin)
			tempMin = tempArrayRAC[i];
		if (tempArrayRAC[i] > tempMax)
			tempMax = tempArrayRAC[i];
	}
	Iacr = tempMax - tempMin;
	
	tempSumm = 0.0;
	for (uint8_t i = 0; i < POINT_COUNT; i++)
	{
		tempSumm += tempArrayIRDC[i];
	}
	Idciravg = tempSumm / (double)POINT_COUNT;
	
	tempSumm = 0.0;
	for (uint8_t i = 0; i < POINT_COUNT; i++)
	{
		tempSumm += tempArrayRDC[i];
	}
	Idcravg = tempSumm / (double)POINT_COUNT;
	
	tempSumm = 0.0;
	for (uint8_t i = 0; i < POINT_COUNT; i++)
	{
		tempSumm += tempArrayIRDC[i] * tempArrayIRDC[i];
	}
	Idcir = sqrt(tempSumm / (double)POINT_COUNT);
	
	tempSumm = 0.0;
	for (uint8_t i = 0; i < POINT_COUNT; i++)
	{
		tempSumm += tempArrayRDC[i] * tempArrayRDC[i];
	}
	Idcr = sqrt(tempSumm / (double)POINT_COUNT);
	
	tempSumm = 0.0;
	for (uint8_t i = 0; i < POINT_COUNT; i++)
	{
		tempSumm += (tempArrayIRDC[i] - Idciravg) * (tempArrayIRDC[i] - Idciravg);
	}
	Idcirdev = sqrt(tempSumm / (double)POINT_COUNT);
	
	tempSumm = 0.0;
	for (uint8_t i = 0; i < POINT_COUNT; i++)
	{
		tempSumm += (tempArrayRDC[i] - Idcravg) * (tempArrayRDC[i] - Idcravg);
	}
	Idcrdev = sqrt(tempSumm / (double)POINT_COUNT);
	
	Iacr /= R_Amp_Coeff;
	Iacir /= IR_Amp_Coeff;

	R = (Iacr / Idcr) * (Idcir / Iacr);
	
	Measurements[SPO2].value = R * Measurements[K_SPO2].value + Measurements[B_SPO2].value;
}

ISR (TCD0_OVF_vect, ISR_NOBLOCK)
{
	PulseoximetryLoop();
	/*
	if(bit_is_set(PORTC.OUT, 1))
	{
		PORTC.OUT &= ~(1 << 1); 
	}
	else
	{
		PORTC.OUT |= 1 << 1;
	}
	*/
}