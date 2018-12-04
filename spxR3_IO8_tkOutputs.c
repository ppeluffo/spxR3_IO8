/*
 * spxR3_IO8_tkOutputs.c
 *
 *  Created on: 6 nov. 2018
 *      Author: pablo
 */


#include "spxR3_IO8.h"

static uint8_t outputs;
static void pv_tkOutputs_init(void);

//------------------------------------------------------------------------------------
void tkOutputs(void * pvParameters)
{

( void ) pvParameters;

	// Espero la notificacion para arrancar
	while ( !startTask )
		vTaskDelay( ( TickType_t)( 100 / portTICK_RATE_MS ) );

	pv_tkOutputs_init();

	xprintf_P( PSTR("starting tkOutputs..\r\n\0"));

	// loop
	for( ;; )
	{

		pub_ctl_watchdog_kick(WDG_OUT);

		vTaskDelay( ( TickType_t)( 1000 / portTICK_RATE_MS ) );

		// Siempre reviso las salidas.
		if ( outputs != systemVars.d_outputs ) {
			outputs = systemVars.d_outputs;		// Variable local que representa el estado de las salidas
			xprintf_P( PSTR("tkOutputs: SET.\r\n\0"));
			IO_reflect_DOUTPUTS(outputs);
		}

	}

}

/*
void tkOutputs(void * pvParameters)
{

( void ) pvParameters;
uint32_t ulNotifiedValue;
BaseType_t xResult;

	// Espero la notificacion para arrancar
	while ( !startTask )
		vTaskDelay( ( TickType_t)( 100 / portTICK_RATE_MS ) );

	pv_tkOutputs_init();

	xprintf_P( PSTR("starting tkOutputs..\r\n\0"));

	// loop
	for( ;; )
	{

		pub_ctl_watchdog_kick(WDG_OUT);

		// Espero. Si salgo por timeout no hago nada. Si es por notificacion ajusto las salidas.
		xResult = xTaskNotifyWait( 0x00, 0xffffffff, &ulNotifiedValue, ((TickType_t) 1000 / portTICK_RATE_MS ) );
		if ( xResult == pdTRUE ) {
			if ( ( ulNotifiedValue & TK_DOUTS_READY ) != 0 ) {
				// Fui notificado:
				// Si las salidas cambiaron en el systemVars las reflejo en el outport.
				xprintf_P( PSTR("tkOutputs: Signal rcvd.\r\n\0"));
				if ( outputs != systemVars.d_outputs ) {
					outputs = systemVars.d_outputs;		// Variable local que representa el estado de las salidas
					IO_reflect_DOUTPUTS(outputs);
				}
			}
		}
	}
}
*/

//------------------------------------------------------------------------------------
static void pv_tkOutputs_init(void)
{
	outputs = systemVars.d_outputs;
	IO_reflect_DOUTPUTS(outputs);

}
//------------------------------------------------------------------------------------
void pub_output_load_defaults(void)
{
	systemVars.d_outputs = 0x00;
}
//------------------------------------------------------------------------------------

