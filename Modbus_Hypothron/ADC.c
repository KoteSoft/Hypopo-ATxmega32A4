/*
 * ADC.c
 *
 * Created: 08.10.2013 0:07:40
 *  Author: Слава
 */ 
#include <avr/io.h>
#include "ADC.h"
#include "GlobalConstants.h"
#include "Params.h"
#include <avr/interrupt.h>

volatile uint8_t ADCMuxFlag = 7;
volatile uint8_t ADCPollComplete = 0;

void ADC_Init()
{
	ADCA.CTRLA = 1 << ADC_ENABLE_bp | 1 << ADC_CH0START_bm;
	ADCA.CTRLB = 0 << ADC_CONMODE_bp | 0 << ADC_FREERUN_bp | ADC_RESOLUTION_12BIT_gc;
	ADCA.REFCTRL = ADC_REFSEL_VCC_gc;
	ADCA.EVCTRL = ADC_SWEEP_0_gc | ADC_EVSEL_0123_gc;
	ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
	ADCA.CAL = 0x0444;
	ADCA.CH0.CTRL = ADC_CH_START_bm | ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_SINGLEENDED_gc;
	/*
	ADCA.CH1.CTRL = ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_SINGLEENDED_gc;
	ADCA.CH2.CTRL = ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_SINGLEENDED_gc;
	ADCA.CH3.CTRL = ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_SINGLEENDED_gc;
	*/
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN7_gc;	
	/*
	ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc;
	ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc;
	ADCA.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc;
	*/	
	ADCA.CH0.INTCTRL = ADC_CH_INTLVL_HI_gc | ADC_CH_INTMODE_COMPLETE_gc;
	/*
	ADCA.CH1.INTCTRL = ADC_CH_INTLVL_HI_gc | ADC_CH_INTMODE_COMPLETE_gc;
	ADCA.CH2.INTCTRL = ADC_CH_INTLVL_HI_gc | ADC_CH_INTMODE_COMPLETE_gc;
	ADCA.CH3.INTCTRL = ADC_CH_INTLVL_HI_gc | ADC_CH_INTMODE_COMPLETE_gc;
	*/
	
}

ISR (ADCA_CH0_vect)
{
	if (ADCMuxFlag == 0)
	{
		Measurements[ADC0 + ADCMuxFlag].value = (ADCA.CH0.RES * U_ref) / ADC_resolution - U_ref * 0.05 + savedParameters[OFF_F1].value;
	} 
	else
	{
		Measurements[ADC0 + ADCMuxFlag].value = (ADCA.CH0.RES * U_ref) / ADC_resolution - U_ref * 0.05;
	}
	
	
	if (ADCMuxFlag < 8)
	{
		ADCMuxFlag++;
	} 
	else
	{
		ADCMuxFlag = 0;
	}	
	
	if (ADCMuxFlag == 8)
	{
		ADCA.CH0.MUXCTRL = 9 << 3; //PINn
	}
	else
	{
		ADCA.CH0.MUXCTRL = ADCMuxFlag << 3; //PINn
	}
	
	ADCA.CH0.CTRL |= ADC_CH_START_bm;
}
