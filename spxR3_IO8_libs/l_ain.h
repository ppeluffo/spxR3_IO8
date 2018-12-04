/*
 * l_ain.h
 *
 *  Created on: 29 oct. 2018
 *      Author: pablo
 */

#ifndef SRC_SPXR3_IO8_LIBS_L_AIN_H_
#define SRC_SPXR3_IO8_LIBS_L_AIN_H_

#include "frtos-io.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <avr/pgmspace.h>
#include "l_ina3221.h"
#include "l_printf.h"

//------------------------------------------------------------------------------------

uint16_t ACH_read_channel( uint8_t channel_id );

#endif /* SRC_SPXR3_IO8_LIBS_L_AIN_H_ */
