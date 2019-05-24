/*
 * l_ina3221.h
 *
 *  Created on: 29 oct. 2018
 *      Author: pablo
 */

#ifndef SRC_SPXR3_IO8_LIBS_L_INA3221_H_
#define SRC_SPXR3_IO8_LIBS_L_INA3221_H_


#include "frtos-io.h"
#include "stdint.h"

#include "../spxR3_IO8_libs/l_i2c.h"

//------------------------------------------------------------------------------------

#define CONF_INAS_SLEEP		0x7920
#define CONF_INAS_AVG128	0x7927

#define INA3221_ADDR_U1			0x82
#define INA3221_ADDR_U2			0x80
#define INA3221_ADDR_U3			0x86

#define INA3231_CONF			0x00
#define INA3221_CH1_SHV			0x01
#define INA3221_CH1_BUSV		0x02
#define INA3221_CH2_SHV			0x03
#define INA3221_CH2_BUSV		0x04
#define INA3221_CH3_SHV			0x05
#define INA3221_CH3_BUSV		0x06
#define INA3221_MFID			0xFE
#define INA3221_DIEID			0xFF

#define INA3221_VCC_SETTLE_TIME	500

uint8_t INA_id2busaddr( uint8_t id );
void INA_config( uint8_t ina_id, uint16_t conf_reg_value );

//#define INA_read( dev_id, rdAddress, data, length ) 	I2C_read( INA_id2busaddr(dev_id), rdAddress, data, length );
//#define INA_write( dev_id, wrAddress, data, length ) 	I2C_write( INA_id2busaddr(dev_id), wrAddress, data, length );

int8_t INA_read( uint8_t dev_id, uint32_t rdAddress, char *data, uint8_t length );
int8_t INA_write( uint8_t dev_id, uint32_t wrAddress, char *data, uint8_t length );

#define INA_config_avg128(ina_id)	INA_config(ina_id, CONF_INAS_AVG128 )
#define INA_config_sleep(ina_id)	INA_config(ina_id, CONF_INAS_SLEEP )


//------------------------------------------------------------------------------------


#endif /* SRC_SPXR3_IO8_LIBS_L_INA3221_H_ */
