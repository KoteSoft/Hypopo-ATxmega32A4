/*
 * Fan.h
 *
 * Created: 24.11.2013 20:59:08
 *  Author: Слава
 */ 


#ifndef FAN_H_
#define FAN_H_

#include <stdint.h>

extern float P_Fan;	//разница между текущим содержанием кислорода и заданным
extern float D_Fan;	//скорость изменения содержания кислорода (производная P_Fan)
extern float preMeasP_Fan; //содержание кислорода, измеренное на предыдущем вдохе
extern uint16_t fanTimer; //10 сек. таймер работы вентилятора
extern uint16_t fanTime; //заданное время работы вентилятора
extern uint8_t fanSpeed; //заданная скорость вентилятора

void PWM_Timer2_Init();
uint8_t FanSpeedCalc(float P, float D);
uint16_t FanTimeCalc(float P, float D);

#endif /* FAN_H_ */