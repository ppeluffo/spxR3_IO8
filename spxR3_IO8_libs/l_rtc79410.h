/*
 * l_rtc79410.h
 *
 *  Created on: 29 oct. 2018
 *      Author: pablo
 */

#ifndef SRC_SPXR3_IO8_LIBS_L_RTC79410_H_
#define SRC_SPXR3_IO8_LIBS_L_RTC79410_H_

#include "frtos-io.h"
#include "stdint.h"
#include "l_i2c.h"

//--------------------------------------------------------------------------------
// API START

typedef struct
{
	// Tamanio: 7 byte.
	// time of day
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	// date
	uint8_t day;
	uint8_t month;
	uint16_t year;

} RtcTimeType_t;

//#define RTC_read( rdAddress, data, length ) I2C_read( BUSADDR_RTC_M79410, rdAddress, data, length );
//#define RTC_write( wrAddress, data, length ) I2C_write( BUSADDR_RTC_M79410, wrAddress, data, length );

int8_t RTC_read( uint32_t rdAddress, char *data, uint8_t length );
int8_t RTC_write( uint32_t wrAddress, char *data, uint8_t length );

void RTC_start(void);
bool RTC_read_dtime(RtcTimeType_t *rtc);
bool RTC_write_dtime(RtcTimeType_t *rtc);

void RTC_rtc2str(char *str, RtcTimeType_t *rtc);
bool RTC_str2rtc(char *str, RtcTimeType_t *rtc);

// API END
//--------------------------------------------------------------------------------

// Direccion del bus I2C donde esta el RTC79410
#define RTC79410_DEVADDR		   	0xDE

// Direcciones de registros

#define RTC79410_RTCSEC			0x00
#define RTC79410_RTCMIN			0x01
#define RTC79410_RTCHOUR		0x02
#define RTC79410_RTCWKDAY		0x03
#define RTC79410_RTCDATE		0x04
#define RTC79410_RTCMTH			0x05
#define RTC79410_RTCYEAR		0x06
#define RTC79410_CONTROL		0x07

// Direccion base donde comienza la SRA
#define RTC79410_ALM0SEC		0x0A
#define RTC79410_ALM0MIN		0x0B
#define RTC79410_ALM0HOUR		0x0C
#define RTC79410_ALM0WKDAY		0x0D
#define RTC79410_ALM0DATE		0x0E
#define RTC79410_ALM0MTH		0x0F

#define RTC79410_SRAM_INIT			0x20
#define FAT_ADDRESS					0x20


#endif /* SRC_SPXR3_IO8_LIBS_L_RTC79410_H_ */
