/*
 * spxR3_IO8_tkDigital.c
 *
 *  Created on: 7 nov. 2018
 *      Author: pablo
 */

#include "spxR3_IO8.h"

static void pv_tkDigital_init(void);

uint16_t digital_inputs[NRO_DIGITAL_CHANNELS];

//------------------------------------------------------------------------------------
void tkDigital(void * pvParameters)
{
	// C/100ms leo las entradas digitales.
	// Las que estan en 1 sumo 1 al contador.
	// En UTE dividimos las entradas digitales en 2: aquellas que solo miden nivel logico (0..3)
	// y las que miden el tiempo que estan en 1 ( 4..7)
	// En modo SPX solo mido el nivel logico

( void ) pvParameters;
uint32_t waiting_ticks;
TickType_t xLastWakeTime;
uint8_t channel;
uint8_t pin;

	// Espero la notificacion para arrancar
	while ( !startTask )
		vTaskDelay( ( TickType_t)( 100 / portTICK_RATE_MS ) );

	xprintf_P( PSTR("starting tkDigital..\r\n\0"));

	pv_tkDigital_init();

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	// Cuento de a 100ms
	waiting_ticks = (uint32_t)(100) / portTICK_RATE_MS;

	// loop
	for( ;; )
	{

		pub_ctl_watchdog_kick(WDG_DIGI);

		// Cada 100 ms leo las entradas digitales. fmax=10Hz
		//vTaskDelay( ( TickType_t)( 100 / portTICK_RATE_MS ) );
		vTaskDelayUntil( &xLastWakeTime, waiting_ticks );

		for ( channel = 0; channel < NRO_DIGITAL_CHANNELS; channel++ ) {

			pin = IO_read_DIN(channel);		    // Leo el nivel del pin

#ifdef UTE
			if ( channel < 4 ) {
				// D0..D3 son de nivel.
				digital_inputs[channel] = pin;

			} else {
				// D4..D7 son contadores de tiempo High.
				if ( pin == 1 )	{
					// Si esta HIGH incremento en 1 tick.
					digital_inputs[channel]++;	// Si esta HIGH incremento en 1 tick.
				}
			}
#endif

#ifdef SPY
			// En modo SPX todos son de nivel.
			digital_inputs[channel] = pin;
#endif

#ifdef OSE
			// En modo SPX todos son de nivel.
			digital_inputs[channel] = pin;
#endif

		}
	}
}
//------------------------------------------------------------------------------------
static void pv_tkDigital_init(void)
{

uint8_t channel;

	for ( channel = 0; channel < NRO_DIGITAL_CHANNELS; channel++ ) {
		digital_inputs[channel] = 0;
	}
}
//------------------------------------------------------------------------------------
// FUNCIONES PUBLICAS
//------------------------------------------------------------------------------------
void pub_digital_read_inputs( uint16_t d_inputs[], bool clear_ticks_flag )
{

uint8_t channel;

	for ( channel = 0; channel < NRO_DIGITAL_CHANNELS; channel++ ) {

#ifdef SPX
		// Copio
		d_inputs[channel] = digital_inputs[channel];
#endif

#ifdef UTE
		// D0..D3 son de nivel.
		if ( channel < 4 ) {
			d_inputs[channel] = digital_inputs[channel];
		} else {
			// D4..D7 son contadores de tiempo High.
			// Puede haber un drift en el timerpoll lo que generaria un falso error
			// Para evitarlo, controlo que los ticks no puedan ser mayor que el timerpoll
			if ( digital_inputs[channel] > systemVars.timerPoll * 10 ) {
				// Corrijo
				digital_inputs[channel] = systemVars.timerPoll * 10;
			}

			// Copio
			d_inputs[channel] = digital_inputs[channel];
		}
#endif

		// Pongo a 0
		if ( clear_ticks_flag ) {
			digital_inputs[channel] = 0;
		}

	}

}
//------------------------------------------------------------------------------------
void pub_digital_load_defaults(void)
{

	// Realiza la configuracion por defecto.

uint8_t channel;

	for ( channel = 0; channel < NRO_DIGITAL_CHANNELS; channel++) {
		snprintf_P( systemVars.d_ch_name[channel], PARAMNAME_LENGTH, PSTR("D%d\0"),channel );
	}

}
//------------------------------------------------------------------------------------
bool pub_data_config_digital_channel( uint8_t channel, char *s_name )
{

bool retS = false;

	while ( xSemaphoreTake( sem_SYSVars, ( TickType_t ) 5 ) != pdTRUE )
		taskYIELD();

	// Controlo que los parametros sean correctos.
	// Channel ID
	if ( channel > 7 ) goto EXIT;

	// NOMBRE
	pub_control_string(s_name);
	snprintf_P( systemVars.d_ch_name[channel], PARAMNAME_LENGTH, PSTR("%s\0"), s_name );

	retS = true;

EXIT:
	xSemaphoreGive( sem_SYSVars );
	return(retS);
}
//------------------------------------------------------------------------------------

