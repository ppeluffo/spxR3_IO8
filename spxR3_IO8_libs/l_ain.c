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
char res[3];
int8_t xBytes;

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

//	xprintf_P( PSTR("ACH: ch=%d, ina=%d, reg=%d, RES1=0x%x, RES0=0x%x\r\n\0") ,channel_id, ina_id, ina_reg, res[1], res[0] );

	an_raw_val = 0;
	an_raw_val = ( res[1]<< 8 ) + res[0];
	an_raw_val = an_raw_val >> 3;

	return( an_raw_val );
}
//------------------------------------------------------------------------------------



