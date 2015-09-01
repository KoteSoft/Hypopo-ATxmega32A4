/*
 * GlobalConstants.c
 *
 * Created: 29.10.2013 11:15:17
 *  Author: Слава
 */ 
#include "GlobalConstants.h"

const float O2_in_air = 20.9;
const float U_ref = 2.0;
const uint16_t H_Step = 1;
const float U0 = 0.0;//0.50
const float Rho = 1.2;
const float ADC_resolution = 4096.0;
const float R_Amp_Coeff = 6.09;
const float IR_Amp_Coeff = 5.30769;
const float HR_Min = 50.0;
const float HR_Max = 200.0;
const float Pulseox_fs = 100.0;
const float Pulseox_fc = 3.0;//4.0
const float Pulseox_RC = 350.0;//350
const float K_IR = 7.1/7.2;
const float SpO2_K = -9.622879;
const float SpO2_B = 108.0861344;
const float hr_dev_limit = 15.0;
const float spo2_dev_limit = 0.2;
const float flow_divider = 5.076142;