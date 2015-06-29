/*
 * Timer0.c
 *
 * Created: 05.10.2013 22:38:59
 *  Author: Слава
 */ 
#include "Timer1.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Params.h"
#include "ADC.h"
#include "GlobalConstants.h"
#include "Flow.h"
#include "Fan.h"
#include <stdint.h>

uint64_t loops;	//количество тактов в интервале	
volatile uint32_t overflows;	//необходимое количество переполнений таймера

void Timer1_Tick();	

ISR (TCE0_OVF_vect)
{	
	Timer1_Tick();	
}

uint8_t Timer1_Init(uint32_t interval)
{
	TCE0.CTRLA = 1 << TC1_CLKSEL2_bp;
	TCE0.PER = (uint16_t)(F_CPU / 8 * interval);
	TCE0.INTCTRLA = 1 << TC1_OVFINTLVL0_bp | 1 << TC1_OVFINTLVL1_bp;
	
	return 0;
}

void Timer1_Tick()
{
	fanTimer++;
	if (fanTimer > (uint16_t)savedParameters[FAN_PID_T].value) //10sec
	{
		fanTimer = 0;
		preMeasP_Fan = P_Fan;
		P_Fan = nonsavedParameters[O2_SET].value - Measurements[O2].value;
		D_Fan = P_Fan - preMeasP_Fan;
		fanSpeed = FanSpeedCalc(P_Fan, D_Fan);
		fanTime = FanTimeCalc(P_Fan, D_Fan);
	} 
	
	if (fanTime > fanTimer)
	{
		TCC0.CCA = 0x7FFF;	//FAN PWM 100%
	}
	else
	{
		TCC0.CCA = 0x0000;	//FAN PWM 0%
	}	
	
	breathDirectionPre = breathDirection;
	if (fabs(Measurements[FlowT].value) > savedParameters[MINBR_FT].value)	//определяем вдох или выдох
	{
		if (Measurements[FlowT].value > 0.0)
		{
			breathDirection = 1;	//вдох
		} 
		else
		{
			breathDirection = -1;	//выдох
		}
	}
	else
	{
		breathDirection = 0;	//нет дыхания
	}
	
	if ((breathDirection == breathDirectionPre))// && breathDirection)	//Продолжается вдох/выдох
	{
		flowIntSum += ((Qprev1 + Measurements[FlowT].value) / 2.0) * (float)H_Step;
		breathTimer++;
		
		/*
		if ((breathDirection == 1) && (fanTimer > 0))
		{
			OCR2 = fanSpeed;
			fanTimer--;
		}
		else
		{
			OCR2 = 0;
		}
		*/		
	} 
	else if (breathDirectionPre != 0) //вдох/выдох закончился
	{
		if (breathDirectionPre > 0)	//закончился вдох
		{
			if ((flowIntSum > savedParameters[BR_V_MIN].value) && (flowIntSum < savedParameters[BR_V_MAX].value))
			{
				Measurements[Vin].value = flowIntSum;
			}
		} 
		else	//закончился выдох
		{
			if ((-flowIntSum > savedParameters[BR_V_MIN].value) && (-flowIntSum < savedParameters[BR_V_MAX].value))
			{
				Measurements[Vout].value = flowIntSum;
			
			
				if ((breathTimer > savedParameters[BR_T_MIN].value) && (breathTimer < savedParameters[BR_T_MAX].value)) //по оканчании вдоха, измеряем период дыхания
				{				
					Measurements[Fbreth].value = 60.0 / ((float)breathTimer / 100.0);
				}
				breathTimer = 0;
			}
			/*Вначале вдоха расччитываем параметры вентилятора*/
			/*
			P_Fan = nonsavedParameters[O2_SET].value - Measurements[O2].value;
			D_Fan = preMeasP_Fan - P_Fan;
			fanSpeed = FanSpeedCalc(P_Fan, D_Fan);
			fanTimer = FanTimeCalc(P_Fan, D_Fan);
			*/
		}
		flowIntSum = 0.0;
	}
	//OCR2 = savedParameters[IT_FAN].value;			
}