/*
 * Flow.h
 *
 * Created: 20.10.2013 19:02:36
 *  Author: Слава
 */ 


#ifndef FLOW_H_
#define FLOW_H_

#include <stdint.h>

extern float Qprev1, Qprev2; //значения ОРВ в предыдущем измерении 
extern int8_t breathDirection, breathDirectionPre; //+1вдох/-1выдох
extern float flowIntSum; //интегральная сумма потока (объем)
extern uint16_t breathTimer; //период вдох-выдох в H_Step
extern int8_t breathDirectionStack[3];

float Out1Calc(float A);
float Out2Calc(float A);
void FlowCalc();

#endif /* FLOW_H_ */