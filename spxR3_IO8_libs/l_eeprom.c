/*
 * l_eeprom.c
 *
 *  Created on: 22 may. 2019
 *      Author: pablo
 */


#include "l_i2c.h"
#include "l_eeprom.h"
#include "l_mcp23018.h"
#include "l_ina3221.h"
#include "l_rtc79410.h"

//------------------------------------------------------------------------------------
int8_t EE_read( uint32_t rdAddress, char *data, uint8_t length )
{

int8_t rcode;
uint8_t times = 3;

	while ( times-- > 0 ) {

		rcode =  I2C_read( BUSADDR_EEPROM_M2402, rdAddress, data, length );

		if ( rcode == -1 ) {
			// Hubo error: trato de reparar el bus y reintentar la operacion
			// Espero 1s que se termine la fuente de ruido.
			vTaskDelay( ( TickType_t)( 1000 / portTICK_RATE_MS ) );
			// Reconfiguro los dispositivos I2C del bus que pueden haberse afectado
			xprintf_P(PSTR("ERROR: EE_read recovering i2c bus (%d)\r\n\0"), times );
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
int8_t EE_write( uint32_t wrAddress, char *data, uint8_t length )
{

int8_t rcode;
uint8_t times = 3;

	while ( times-- > 0 ) {

		rcode =  I2C_write( BUSADDR_EEPROM_M2402, wrAddress, data, length );

		if ( rcode == -1 ) {
			// Hubo error: trato de reparar el bus y reintentar la operacion
			// Espero 1s que se termine la fuente de ruido.
			vTaskDelay( ( TickType_t)( 1000 / portTICK_RATE_MS ) );
			// Reconfiguro los dispositivos I2C del bus que pueden haberse afectado
			xprintf_P(PSTR("ERROR: EE_write recovering i2c bus (%d)\r\n\0"), times );
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
