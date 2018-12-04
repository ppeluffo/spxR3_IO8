/*
 * l_ina3221.c
 *
 *  Created on: 29 oct. 2018
 *      Author: pablo
 */


#include "l_ina3221.h"

//------------------------------------------------------------------------------------
uint8_t INA_id2busaddr( uint8_t id )
{
	switch(id) {
	case 0:
		return(INA3221_ADDR_U1);	// Canales 0,1,2
		break;
	case 1:
		return(INA3221_ADDR_U2); // Canales 3,4,5
		break;
	case 2:
		return(INA3221_ADDR_U3); // Canales 3,4,5
		break;
	default:
		return(99);
		break;

	}

	return(99);

}
//------------------------------------------------------------------------------------
void INA_config( uint8_t ina_id, uint16_t conf_reg_value )
{
char res[3];
int8_t xBytes;

	res[0] = ( conf_reg_value & 0xFF00 ) >> 8;
	res[1] = ( conf_reg_value & 0x00FF );
	xBytes = INA_write( ina_id, INA3231_CONF, res, 2 );
	if ( xBytes == -1 )
		xprintf_P(PSTR("ERROR: I2C:INA_config\r\n\0"));

}
//------------------------------------------------------------------------------------


