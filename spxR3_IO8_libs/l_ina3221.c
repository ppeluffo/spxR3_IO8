/*
 * l_ina3221.c
 *
 *  Created on: 29 oct. 2018
 *      Author: pablo
 */


#include "l_ina3221.h"
#include "l_mcp23018.h"
#include "l_rtc79410.h"

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
int8_t INA_read( uint8_t dev_id, uint32_t rdAddress, char *data, uint8_t length )
{

int8_t rcode;
uint8_t times = 3;

	while ( times-- > 0 ) {

		rcode =  I2C_read( INA_id2busaddr(dev_id), rdAddress, data, length );

		if ( rcode == -1 ) {
			// Hubo error: trato de reparar el bus y reintentar la operacion
			// Espero 1s que se termine la fuente de ruido.
			vTaskDelay( ( TickType_t)( 1000 / portTICK_RATE_MS ) );
			// Reconfiguro los dispositivos I2C del bus que pueden haberse afectado
			xprintf_P(PSTR("ERROR: INA(%d)_read recovering i2c bus (%d)\r\n\0"), dev_id, times );
			MCP_init();
			INA_config_avg128(0);
			INA_config_avg128(1);
			INA_config_avg128(2);
			RTC_start();
		} else {
			// No hubo error: salgo normalmente
			break;
		}
	}
	return( rcode );

}
//------------------------------------------------------------------------------------
int8_t INA_write( uint8_t dev_id, uint32_t wrAddress, char *data, uint8_t length )
{

int8_t rcode;
uint8_t times = 3;

	while ( times-- > 0 ) {

		rcode =  I2C_write( INA_id2busaddr(dev_id), wrAddress, data, length );

		if ( rcode == -1 ) {
			// Hubo error: trato de reparar el bus y reintentar la operacion
			// Espero 1s que se termine la fuente de ruido.
			vTaskDelay( ( TickType_t)( 1000 / portTICK_RATE_MS ) );
			// Reconfiguro los dispositivos I2C del bus que pueden haberse afectado
			xprintf_P(PSTR("ERROR: INA(%d)_write recovering i2c bus (%d)\r\n\0"), dev_id, times );
			MCP_init();
			INA_config_avg128(0);
			INA_config_avg128(1);
			INA_config_avg128(2);
			RTC_start();
		} else {
			// No hubo error: salgo normalmente
			break;
		}
	}
	return( rcode );

}
//------------------------------------------------------------------------------------


