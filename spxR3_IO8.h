/*
 * spx.h
 *
 *  Created on: 20 de oct. de 2016
 *      Author: pablo
 */

#ifndef SRC_SPXR1_H_
#define SRC_SPXR1_H_

//------------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------------
#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <compat/deprecated.h>
#include <avr/pgmspace.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <avr/sleep.h>
#include <ctype.h>
#include <avr/wdt.h>
#include "avr_compiler.h"
#include "clksys_driver.h"
#include <inttypes.h>

#include "TC_driver.h"
#include "pmic_driver.h"
#include "wdt_driver.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "croutine.h"
#include "semphr.h"
#include "timers.h"
#include "limits.h"

#include "frtos-io.h"

#include "l_iopines.h"
#include "l_eeprom.h"
#include "l_nvm.h"
#include "l_ina3221.h"
#include "l_rtc79410.h"
#include "l_file.h"
#include "l_printf.h"
#include "l_ain.h"
#include "l_mcp23018.h"

//------------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------------
#define SPX_FW_REV "0.0.7"
#define SPX_FW_DATE "@ 20181204"

#define SPX_HW_MODELO "spxR3_IO8 HW:xmega256A3B R1.0"
#define SPX_FTROS_VERSION "FW:FRTOS10"

#define NRO_ANALOG_CHANNELS		8
#define NRO_DIGITAL_CHANNELS	8
#define NRO_COUNTERS_CHANNELS	2
#define NRO_DIGITAL_OUTPUTS		8

//#define UTE
#ifndef UTE
	#define SPX
#endif

#define F_CPU (32000000UL)

//#define SYSMAINCLK 2
//#define SYSMAINCLK 8
#define SYSMAINCLK 32

#define CHAR32	32
#define CHAR64	64
#define CHAR128	128
#define CHAR256	256

#define tkCtl_STACK_SIZE		512
#define tkCmd_STACK_SIZE		512
#define tkData_STACK_SIZE		512
#define tkCounters_STACK_SIZE	512
#define tkDigital_STACK_SIZE	512
#define tkGprs_rx_STACK_SIZE	1024
#define tkGprs_tx_STACK_SIZE	1024
#define tkOutputs_STACK_SIZE	512

#define tkXbee_STACK_SIZE		512

#define tkCtl_TASK_PRIORITY	 		( tskIDLE_PRIORITY + 1 )
#define tkCmd_TASK_PRIORITY	 		( tskIDLE_PRIORITY + 1 )
#define tkData_TASK_PRIORITY	 	( tskIDLE_PRIORITY + 1 )
#define tkCounters_TASK_PRIORITY	( tskIDLE_PRIORITY + 1 )
#define tkDigital_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define tkGprs_rx_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define tkGprs_tx_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define tkOutputs_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

#define tkXbee_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

#define DLGID_LENGTH		10
#define PARAMNAME_LENGTH	5
#define IP_LENGTH			24
#define APN_LENGTH			32
#define PORT_LENGTH			7
#define SCRIPT_LENGTH		64
#define PASSWD_LENGTH		15
#define PARAMNAME_LENGTH	5

TaskHandle_t xHandle_idle, xHandle_tkCtl,xHandle_tkCmd, xHandle_tkData, xHandle_tkDigital, xHandle_tkCounters, xHandle_tkGprsRx, xHandle_tkGprsTx, xHandle_tkOutputs, xHandle_tkXbee;

char stdout_buff[CHAR64];

// Mensajes entre tareas
#define TK_FRAME_READY			0x01	//
#define TK_DOUTS_READY			0x02	//

//------------------------------------------------------------------------------------
typedef enum { DEBUG_NONE = 0, DEBUG_GPRS, DEBUG_COUNT } t_debug;
typedef enum { USER_NORMAL, USER_TECNICO } usuario_t;
//------------------------------------------------------------------------------------

// Estructura para manejar la hora de aplicar las consignas
typedef struct {
	uint8_t hour;
	uint8_t min;
} time_t;

// Estructura de datos manejados por la tarea DATA = ANALOGICO + DIGITAL + COUNTERS.
typedef struct {
	RtcTimeType_t rtc;								//  7
	float analog_data[NRO_ANALOG_CHANNELS];			// 32
	uint16_t digital_data[NRO_DIGITAL_CHANNELS];	// 16
	float counters_data[NRO_COUNTERS_CHANNELS];		//  8
} st_data_frame;

typedef struct {
	// Variables de trabajo.

	char dlgId[DLGID_LENGTH];
	char apn[APN_LENGTH];
	char server_tcp_port[PORT_LENGTH];
	char server_ip_address[IP_LENGTH];
	char dlg_ip_address[IP_LENGTH];
	char serverScript[SCRIPT_LENGTH];
	char passwd[PASSWD_LENGTH];

	// Configuracion de Canales analogicos
	uint16_t coef_calibracion[NRO_ANALOG_CHANNELS];
	uint8_t imin[NRO_ANALOG_CHANNELS];	// Coeficientes de conversion de I->magnitud (presion)
	uint8_t imax[NRO_ANALOG_CHANNELS];
	float mmin[NRO_ANALOG_CHANNELS];
	float mmax[NRO_ANALOG_CHANNELS];
	char an_ch_name[NRO_ANALOG_CHANNELS][PARAMNAME_LENGTH];
//	char an_ch_modo[NRO_ANALOG_CHANNELS];		// Local o remoto

	// Configuracion de canales digitales
	// Niveles logicos
	char d_ch_name[NRO_DIGITAL_CHANNELS][PARAMNAME_LENGTH];

	char c_ch_name[NRO_COUNTERS_CHANNELS][PARAMNAME_LENGTH];
	float c_ch_magpp[NRO_COUNTERS_CHANNELS];

	uint8_t d_outputs;

	uint16_t timerPoll;

	uint8_t csq;
	uint8_t dbm;
	t_debug debug;

//	t_modoXbee xbee;

	// El checksum DEBE ser el ultimo byte del systemVars !!!!
	uint8_t checksum;

} systemVarsType;

systemVarsType systemVars;

bool startTask;
//------------------------------------------------------------------------------------
// PROTOTIPOS
//------------------------------------------------------------------------------------
void tkCtl(void * pvParameters);
void tkCmd(void * pvParameters);
void tkData(void * pvParameters);
void tkCounters(void * pvParameters);
void tkDigital(void * pvParameters);
void tkGprsRx(void * pvParameters);
void tkGprsTx(void * pvParameters);
void tkOutputs(void * pvParameters);

void tkXbee(void * pvParameters);

xSemaphoreHandle sem_SYSVars;
StaticSemaphore_t SYSVARS_xMutexBuffer;
#define MSTOTAKESYSVARSSEMPH ((  TickType_t ) 10 )

// Utils
void u_configure_systemMainClock(void);
void u_configure_RTC32(void);
void initMCU(void);
void pub_load_defaults (void );
uint8_t pub_save_params_in_NVMEE(void);
bool u_load_params_from_NVMEE(void);
void u_convert_str_to_time_t ( char *time_str, time_t *time_struct );
void u_convert_int_to_time_t ( int int16time, time_t *time_struct );
void pub_convert_str_to_time_t ( char *time_str, time_t *time_struct );
void pub_control_string( char *s_name );

// tkCtl
void pub_ctl_watchdog_kick(uint8_t taskWdg );
void pub_ctl_print_wdg_timers(void);
void pub_ctl_print_stack_watermarks(void);
uint16_t pub_ctl_readTimeToNextPoll(void);
void pub_ctl_reload_timerPoll(void);

// tkCounters
void pub_counters_read( uint16_t *count0, uint16_t *count1, bool clear_counters_flag );
bool pub_counter_config_channel( uint8_t channel, char *s_param0, char *s_param1 );
void pub_counter_load_defaults(void);

// tkDigital
void pub_digital_read_inputs( uint16_t d_inputs[], bool clear_ticks_flag );
void pub_digital_load_defaults(void);
bool pub_data_config_digital_channel( uint8_t channel, char *s_name );

// tkData
void pub_data_read_frame(bool wdg_control);
void pub_data_print_frame(bool wdg_control);
void pub_data_read_anChannel ( uint8_t channel, uint16_t *raw_val, float *mag_val );
void pub_data_config_timerpoll ( char *s_timerpoll );
bool pub_data_config_analog_channel( uint8_t channel,char *s_aname,char *s_imin,char *s_imax,char *s_mmin,char *s_mmax );
void pub_data_load_defaults(void);

// tkGprs
void pub_gprs_load_defaults(void);

// tkOutputs
void pub_output_load_defaults(void);

// WATCHDOG
#define WDG_CMD			0
#define WDG_CTL			1
#define WDG_COUNT		2
#define WDG_DAT			3
#define WDG_OUT			4
#define WDG_GPRSRX		5
#define WDG_GPRSTX		6
#define WDG_DIGI		7

#define NRO_WDGS		2

uint8_t wdg_resetCause;

#endif /* SRC_SPXR1_H_ */
