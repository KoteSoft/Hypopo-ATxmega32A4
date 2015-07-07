/*
 * Modbus_Hypothron.c
 *
 * Created: 10.09.2013 17:53:13
 *  Author: Слава
 */ 

/*
TCC0 - ШИМ компрессор, динамик, подсветка
TCC1 - Пульсоксиметр RIR
TCD0 - Пульсоксиметр Loop
TCD1 - Modbus
TCE0 - FlowCalc
*/

#include "main.h"
#include "Timer1.h"
#include "ADC.h"
#include "Params.h"
#include "GlobalConstants.h"
#include "Flow.h"
#include "Fan.h"
#include "Core.h"
#include "Pulseoximetry.h"

USHORT   usRegInputBuf[REG_INPUT_NREGS];
USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];

unsigned char ucRegCoilsBuf[REG_COILS_SIZE / 8];

int main(void)
{
	coreInit();
	_delay_ms(500);
		
	Qprev1 = 0.0;
	Qprev2 = 0.0;
	Measurements[STATE].array[0] = DEVICE_IDLE_STATE;	
	
	ADC_Init();
	Timer1_Init(H_Step);
	PWM_Timer2_Init();	
	PulseoximetryInit();
	
	/*Настраиваем Modbus*/
	eMBErrorCode eStatus = eMBInit( MB_RTU, 0x01, 0, UART_BAUD_RATE, MB_PAR_NONE );
	eStatus = eMBEnable();
	
	sei();
	
	/*Загружаем в Holding Registers и в массив параметров значения из EEPROM*/
	ModbusInitValues();
	
	//Sound_StartMusic();
	
	while(1)
	{
		/*Актуализируем значения Modbus-регистров в соответствии со значениями параметров*/
		ModbusLoader();
		/*Актуализируем значения параметров в соответствии со значениями Holding Registers*/
		ModbusSaver();
		//ADC_Poll();
		HugeCalculations();
		/*Итерация Modbus*/
		eMBPoll();	
	}
	
}

eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
	eMBErrorCode    eStatus = MB_ENOERR;
	int             iRegIndex;

	if( ( usAddress >= REG_INPUT_START )
	&& ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
	{
		iRegIndex = ( int )( usAddress - usRegInputStart );
	        while( usNRegs > 0 )
	        {
			*pucRegBuffer++ =
			( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
			*pucRegBuffer++ =
			( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
			iRegIndex++;
			usNRegs--;
		}
	}
	else
	{
		eStatus = MB_ENOREG;
	}

	return eStatus;
}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_HOLDING_START ) &&
    ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        switch ( eMode )
        {
            /* Pass current register values to the protocol stack. */
            case MB_REG_READ:
            while( usNRegs > 0 )
            {
                *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] >> 8 );
                *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;

            /* Update current register values with new values from the
            * protocol stack. */
            case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}



eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
eMBRegisterMode eMode )
{
    return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    return MB_ENOREG;
}
