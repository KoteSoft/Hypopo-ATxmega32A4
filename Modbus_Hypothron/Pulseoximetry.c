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
#define ARRAY_HR_AVG_SIZE 5
#define ARRAY_SPO2_AVG_SIZE 1000
#define ARRAY_SPO2_INT_SIZE 4
#define ADC_R_AC ADC4
#define ADC_IR_AC ADC6
#define ADC_R_DC ADC3
#define ADC_IR_DC ADC5

float arrayIRAC[POINT_COUNT];
float arrayIRDC[POINT_COUNT];
//float arrayRAC[POINT_COUNT];
float arrayRDC[POINT_COUNT];
float arrayFilter[FILTER_POINT_COUNT];

float arrayHrAvg[ARRAY_HR_AVG_SIZE];
float arraySpo2Int[ARRAY_SPO2_INT_SIZE];
volatile float spo2AvgSumm = 0;

volatile int8_t pointIndex = 0;		//Индекс текущего перезаписываемого элемента буфферов пульсоксиметрии
volatile uint16_t loopCounter = 0;	//Счетчик итераций пульсоксиметрии для расчета периодов ЧСС
volatile uint8_t hrAvgIndex = 0;
volatile uint16_t spo2AvgIndex = 0;
volatile uint8_t spo2IntIndex = 0;
volatile uint8_t hrPeakFindFlag = 0;

struct butterwrthParams_t pulseoxButterworthSettings;

inline void PulseoximetryLoop();

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
	TCC1.CTRLA = TC_CLKSEL_DIV8_gc;
	TCC1.CTRLB = TC1_CCBEN_bm | TC_WGMODE_SS_gc;
	TCC1.PER = 0xFFFF;
	TCC1.CCB = TCC1.PER / 2;
	//PORTC.DIR |= 1 << 5;
	//PORTC.OUT |= 1 << 5; 
	
	
	//Таймер опроса АЦП пульсоксиметра - 100Гц
	TCD0.CTRLA = TC_CLKSEL_DIV256_gc;
	TCD0.CTRLB = TC_WGMODE_NORMAL_gc;
	TCD0.PER = 1250;
	TCD0.INTCTRLA = TC_OVFINTLVL_LO_gc;
	
	//Рассчет коэффициентов фильтра Баттерворта ФПГ
	float w = tan((M_PI * Pulseox_fc) / Pulseox_fs);
	float c = 1.0 + 2 * cos(M_PI / 4.0) * w + w * w;
	pulseoxButterworthSettings.b2 = (1.0 - 2.0 * cos(M_PI / 4.0) * w + w * w) / c;
	pulseoxButterworthSettings.b1 = (2.0 * (w * w - 1)) / c;
	pulseoxButterworthSettings.a2 = (w * w) / c;
	pulseoxButterworthSettings.a0 = pulseoxButterworthSettings.a2;
	pulseoxButterworthSettings.a1 = 2 * pulseoxButterworthSettings.a0;
}

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
		hrPeakFindFlag = 1;
	}	
	
	loopCounter++;
	pointIndex++;
}

void PulseoximetryHugeCalculation()
{
	volatile float Iacir, Iacr, Idcir, Idcr, R;
	volatile float tempMin, tempMax, tempSumm;
	float tempArrayIRDC[POINT_COUNT];
	float tempArrayRDC[POINT_COUNT];
	
	//Создаем копии буфферов пульсоксиметрии для предотвращения их перезаписи во время расчета                                                                                                                           
	for(uint8_t i = 0; i < POINT_COUNT; i++)
	{
		tempArrayIRDC[i] = (arrayIRDC[i]) * K_IR;
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
	Idcir = tempMin;
	
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
	Idcr = tempMin;
	
	R = log(Idcr / Iacr) / log(Idcir / Iacir);
	//if (R > 0.5 && R < 2.0)
	{
		spo2AvgIndex++;
		if (spo2AvgIndex >= ARRAY_SPO2_AVG_SIZE)
		{
			Measurements[SPO2_AVG].value = spo2AvgSumm / (float)ARRAY_SPO2_AVG_SIZE;
			arraySpo2Int[spo2IntIndex] = spo2AvgSumm / (float)ARRAY_SPO2_AVG_SIZE;
			spo2IntIndex++;
			if (spo2IntIndex >= ARRAY_SPO2_INT_SIZE)
			{
				spo2IntIndex = 0;
			}
			tempSumm = 0.0f;
			for(uint8_t i = 0; i < ARRAY_SPO2_INT_SIZE; i++)
			{
				tempSumm += arraySpo2Int[i];
			}
			tempSumm = (int)(tempSumm / (float)ARRAY_SPO2_INT_SIZE);
			if (tempSumm > 99.5)
			{
				tempSumm = 99.0;
			}
			Measurements[SPO2].value = tempSumm;
			spo2AvgSumm = 0.0;
			spo2AvgIndex = 0;
			
		}
		spo2AvgSumm += R * savedParameters[K_SPO2].value + savedParameters[B_SPO2].value;	
	}
}

void HeartRateHugeCalculation()
{
	if (hrPeakFindFlag)
	{
		hrPeakFindFlag = 0;
		if (HR_Min < (float)((Pulseox_fs / loopCounter) * 60.0) && HR_Max > (float)((Pulseox_fs / loopCounter) * 60.0))
		{
			hrAvgIndex++;
			if (hrAvgIndex >= ARRAY_HR_AVG_SIZE)
			{
				hrAvgIndex = 0;
			}
			arrayHrAvg[hrAvgIndex] = (float)((Pulseox_fs / loopCounter) * 60.0);
			float summ = 0.0f;
			for (uint8_t i = 0; i < ARRAY_HR_AVG_SIZE; i++)
			{
				summ += arrayHrAvg[i];
			}
			Measurements[HR_AVG].value = summ / (float)ARRAY_HR_AVG_SIZE;
		
			summ = 0.0f;
			for (uint8_t i = 0; i < ARRAY_HR_AVG_SIZE; i++)
			{
				summ += (arrayHrAvg[i] - Measurements[HR_AVG].value) * (arrayHrAvg[i] - Measurements[HR_AVG].value);
			}
			Measurements[HR_DEV].value = sqrt(summ / (float)ARRAY_HR_AVG_SIZE);
		
			if (Measurements[HR_DEV].value < hr_dev_limit)
			{
				Measurements[HR].value = Measurements[HR_AVG].value;
			}
		}
	loopCounter = 0;
	}
}

ISR (TCD0_OVF_vect)
{
	PulseoximetryLoop();
}