/*
 * GlobalConstants.h
 *
 * Created: 29.10.2013 11:12:59
 *  Author: Слава
 */ 


#ifndef GLOBALCONSTANTS_H_
#define GLOBALCONSTANTS_H_

#include <stdint.h>

extern const float O2_in_air; //%кислорода в воздухе
extern const float U_ref; //опорное напряжение АЦП	
extern const uint16_t H_Step; //время дискретизации
extern const float U0; //Напряжение на выходе усилителя ОРВ при отсутствии потока
extern const float Rho; //Плотность воздуха
extern const float ADC_resolution; //Разрешение АЦП
extern const float R_Amp_Coeff; //
extern const float IR_Amp_Coeff; //
extern const float HR_Min;
extern const float HR_Max;
extern const float Pulseox_fs; //Частота выборки данных пульсоксиметра
extern const float Pulseox_fc; //Частота среза данных пульсоксиметра
extern const float Pulseox_RC;
extern const float K_IR;
extern const float SpO2_K; 
extern const float SpO2_B;
extern const float hr_dev_limit;
extern const float spo2_dev_limit;
extern const float flow_divider;
#endif /* GLOBALCONSTANTS_H_ */