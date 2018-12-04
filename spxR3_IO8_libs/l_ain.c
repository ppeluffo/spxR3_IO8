/*
 * l_ain.c
 *
 *  Created on: 29 oct. 2018
 *      Author: pablo
 */


#include "l_ain.h"

//------------------------------------------------------------------------------------
uint16_t ACH_read_channel( uint8_t channel_id )
{

uint8_t ina_reg = 0;
uint8_t ina_id = 0;
uint16_t an_raw_val;
uint8_t MSB, LSB;
char res[3];
int8_t xBytes;
float vshunt;

	switch ( channel_id ) {
	case 0:
		ina_id = 0; ina_reg = INA3221_CH1_SHV;break;
	case 1:
		ina_id = 0; ina_reg = INA3221_CH2_SHV;break;
	case 2:
		ina_id = 0; ina_reg = INA3221_CH3_SHV;break;
	case 3:
		ina_id = 1; ina_reg = INA3221_CH1_SHV;break;
	case 4:
		ina_id = 1; ina_reg = INA3221_CH2_SHV;break;
	case 5:
		ina_id = 1; ina_reg = INA3221_CH3_SHV;break;
	case 6:
		ina_id = 2; ina_reg = INA3221_CH1_SHV;break;
	case 7:
		ina_id = 2; ina_reg = INA3221_CH2_SHV;break;
	case 8:
		ina_id = 2; ina_reg = INA3221_CH3_SHV;break;
	default:
		return(-1);
	}

	// Leo el valor del INA.
	xBytes = INA_read( ina_id, ina_reg, res ,2 );
	if ( xBytes == -1 )
		xprintf_P(PSTR("ERROR: I2C:INA:ACH_read_channel\r\n\0"));

	an_raw_val = 0;
	MSB = res[0];
	LSB = res[1];
	an_raw_val = ( MSB << 8 ) + LSB;
	an_raw_val = an_raw_val >> 3;
	vshunt = (float) an_raw_val * 40 / 1000;

//	xprintf_P( PSTR("ACH: ch=%d, ina=%d, reg=%d, MSB=0x%x, LSB=0x%x, ANV=(0x%x)%d, VSHUNT = %.02f(mV)\r\n\0") ,channel_id, ina_id, ina_reg, MSB, LSB, an_raw_val, an_raw_val, vshunt );

	return( an_raw_val );
}
//------------------------------------------------------------------------------------



