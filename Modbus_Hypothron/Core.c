/*
 * Core.c
 *
 * Created: 11.02.2015 23:26:33
 *  Author: Слава
 */ 
#include "Core.h"
#include "avr/io.h"

void coreInit()
{
	//CLOCK
	OSC.CTRL|=OSC_RC32MEN_bm;
	while (!(OSC.STATUS & OSC_RC32MRDY_bm));
		CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_RC32M_gc;
	// If you want to disable RC2M
	OSC.CTRL&=(~OSC_RC2MEN_bm);
	
	//Порты
	PORTC.DIR = 0b10111111;
	
	//Прерывания
	PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_LOLVLEN_bm;
}