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

#define POINT_COUNT 60			//Размер буфферов пульсоксиметрии
#define FILTER_POINT_COUNT 5	//Размер буффера фильтрованной ФПГ
#define ADC_R_AC ADC6
#define ADC_IR_AC ADC4
#define ADC_R_DC ADC3
#define ADC_IR_DC ADC5

float arrayIRAC[POINT_COUNT];
float arrayIRDC[POINT_COUNT];
//float arrayRAC[POINT_COUNT];
float arrayRDC[POINT_COUNT];
float arrayFilter[FILTER_POINT_COUNT];

struct butterwrthParams_t pulseoxButterworthSettings;

void PulseoximetryInit()
{
	//DAC
	/*
	DACB.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
	DACB.CTRLB = DAC_CHSEL_SINGLE_gc;
	DACB.CTRLC = DAC_REFSEL_AVCC_gc;
	DACB.TIMCTRL = DAC_CONINTVAL_128CLK_gc | DAC_REFRESH_16CLK_gc;
	DACB.CH0DATA = 0x0000;
	DACB.CH1DATA = 0x0000;	
	*/	
	PORTB.DIR = 1 << 2 | 1 << 3;
	PORTB.OUT = 1 << 2 | 1 << 3;
	
	//TIMER1 RIR
	TCC1.CTRLA = TC_CLKSEL_DIV1_gc;
	TCC1.CTRLB = TC1_CCBEN_bm | TC_WGMODE_SS_gc;
	TCC1.PER = 0x8FFF;
	TCC1.CCB = 0x4FFF;
	//PORTC.DIR |= 1 << 5;
	//PORTC.OUT |= 1 << 5; 
	
	
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

int8_t pointIndex = 0;		//Индекс текущего перезаписываемого элемента буфферов пульсоксиметрии
uint16_t loopCounter = 0;	//Счетчик итераций пульсоксиметрии для расчета периодов ЧСС

void PulseoximetryLoop()
{
	//Сдвигаем буффер фильтрованной ФПГ
	for (uint8_t i = 0; i < FILTER_POINT_COUNT - 1; i++)
	{
		arrayFilter[i] = arrayFilter[i + 1];
	}	
	
	if (pointIndex >= POINT_COUNT)
	{
		pointIndex = 0;
	}
	
	//Заносим в буфферы последние значения
	arrayIRAC[pointIndex] = Measurements[ADC_IR_AC].value;
	arrayIRDC[pointIndex] = Measurements[ADC_IR_DC].value;
	arrayRDC[pointIndex] = Measurements[ADC_R_DC].value;
		
	float tmpX[3], tmpY[2];
	
	for (uint8_t i = 0; i < 3; i++)
	{
		if ((int8_t)pointIndex - (int8_t)i < 0)
		{
			tmpX[i] = arrayIRAC[POINT_COUNT - 1 - (i - pointIndex)];
		}
		else
		{
			tmpX[i] = arrayIRAC[pointIndex - i];
		}
	}
		
	for (uint8_t i = 0; i < 2; i++)
	{
		tmpY[i] = arrayFilter[FILTER_POINT_COUNT - 2 - i];
	}
	
	//Расчитываем последнее значение фильтрованной ФПГ
	arrayFilter[FILTER_POINT_COUNT - 1] = LowPassFilter(Butterworth2Filter(tmpX, tmpY,pulseoxButterworthSettings), arrayFilter[FILTER_POINT_COUNT - 2], Pulseox_fs, Pulseox_RC);
	//Расчет ЧСС
	if (arrayFilter[FILTER_POINT_COUNT - 1] < arrayFilter[FILTER_POINT_COUNT - 2] && arrayFilter[FILTER_POINT_COUNT - 2] > arrayFilter[FILTER_POINT_COUNT - 3])
	{		
		if (HR_Min < (float)((Pulseox_fs / loopCounter) * 60.0) && HR_Max > (float)((Pulseox_fs / loopCounter) * 60.0))
		{
			Measurements[HR].value = (float)((Pulseox_fs / loopCounter) * 60.0);
		}
		loopCounter = 0;
	}	
	
	loopCounter++;
	pointIndex++;
}

void PulseoximetryHugeCalculation()
{
	float Iacir, Iacr, Idcir, Idcr, Idcrdev, Idcirdev, Idcravg, Idciravg, R;
	float tempMin, tempMax, tempSumm;
	//float tempArrayIRAC[POINT_COUNT];
	float tempArrayIRDC[POINT_COUNT];
	//float tempArrayRAC[POINT_COUNT];
	float tempArrayRDC[POINT_COUNT];
	
	//Создаем копии буфферов пульсоксиметрии для предотвращения их перезаписи во время расчета                                                                                                                           
	for(uint8_t i = 0; i < POINT_COUNT; i++)
	{
		//tempArrayIRAC[i] = arrayIRAC[i];
		tempArrayIRDC[i] = arrayIRDC[i] * K_IR;
		//tempArrayRAC[i] = arrayRAC[i];
		tempArrayRDC[i] = arrayRDC[i];
	}
	
	tempMin = 10.0;
	tempMax = 0.0;
	for (uint8_t i = 0; i < POINT_COUNT; i++)
	{
		if (tempArrayIRDC[i] < tempMin)
			tempMin = tempArrayIRDC[i];
		if (tempArrayIRDC[i] > tempMax)
			tempMax = tempArrayIRDC[i];
	}
	Iacir = tempMax - tempMin;
	
	tempMin = 10.0;
	tempMax = 0.0;
	for (uint8_t i = 0; i < POINT_COUNT; i++)
	{
		if (tempArrayRDC[i] < tempMin)
		tempMin = tempArrayRDC[i];
		if (tempArrayRDC[i] > tempMax)
		tempMax = tempArrayRDC[i];
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
	
	R = (Iacr / Idcr) * (Idcir / Iacir);
	if (R > 0.94 && R < 2.0)
	{
		Measurements[SPO2].value = R * SpO2_K + SpO2_B;
	}
}

ISR (TCD0_OVF_vect)
{
	PulseoximetryLoop();
	//DACB.CH0DATA = savedParameters[K_RIR].array[0];
	//DACB.CH1DATA = savedParameters[K_RIR].array[1];
}