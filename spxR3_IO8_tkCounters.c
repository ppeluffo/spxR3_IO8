/*
 * spxR3_IO8_tkCounters.c
 *
 *  Created on: 29 oct. 2018
 *      Author: pablo
 *
 *
  *
 */

#include "spxR3_IO8.h"

BaseType_t xHigherPriorityTaskWokenDigital = pdFALSE;

static uint16_t counter0, counter1;

static bool wakeup_for_C0, wakeup_for_C1;

static void pv_tkCounters_init(void);

// La tarea puede estar hasta 10s en standby
#define WDG_DIN_TIMEOUT	30

//------------------------------------------------------------------------------------
void tkCounters(void * pvParameters)
{

( void ) pvParameters;
uint32_t ulNotificationValue;
const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 10000 );

	// Espero la notificacion para arrancar
	while ( !startTask )
		vTaskDelay( ( TickType_t)( 100 / portTICK_RATE_MS ) );

	pv_tkCounters_init();

	xprintf_P( PSTR("starting tkCounters..\r\n\0"));

	// loop
	for( ;; )
	{

		pub_ctl_watchdog_kick(WDG_DIN, WDG_DIN_TIMEOUT);

		// Cuando la interrupcion detecta un flanco, solo envia una notificacion
		// Espero que me avisen. Si no me avisaron en 10s salgo y repito el ciclo.
		// Esto es lo que me permite entrar en tickless.
		ulNotificationValue = ulTaskNotifyTake( pdFALSE, xMaxBlockTime );

		if( ulNotificationValue != 0 ) {
			// Fui notificado: llego algun flanco que determino.

			if ( wakeup_for_C0 ) {
				// El contador C0 solo puede estar en D1
				counter0++;
				wakeup_for_C0 = false;
			}

			if ( wakeup_for_C1 ) {
				// El contador C1 solo puede estar en D2
				counter1++;
				wakeup_for_C1 = false;
			}

			if ( systemVars.debug == DEBUG_COUNT) {
				xprintf_P( PSTR("COUNTERS: C0=%d,C1=%d\r\n\0"),counter0,counter1);
			}

			// Espero 100ms de debounced
			vTaskDelay( ( TickType_t)( 100 / portTICK_RATE_MS ) );

			IO_clr_CLRD();		// Borro el latch llevandolo a 0.
			IO_set_CLRD();		// Lo dejo en reposo en 1

		} else   {
			// Expiro el timeout de la tarea. Por ahora no hago nada.
		}
	}
}
//------------------------------------------------------------------------------------
static void pv_tkCounters_init(void)
{
	// Configuracion inicial de la tarea

	// Configuracion de las interrupciones que genera el contador
	// PA2, PB2.
	// Los pines ya estan configurados como entradas.
	//
	PORTA.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_RISING_gc;	// Sensa rising edge
	PORTA.INT0MASK = PIN2_bm;
	PORTA.INTCTRL = PORT_INT0LVL0_bm;

	PORTB.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_RISING_gc;	// Sensa rising edge
	PORTB.INT0MASK = PIN2_bm;
	PORTB.INTCTRL = PORT_INT0LVL0_bm;

	wakeup_for_C0 = false;
	wakeup_for_C1 = false;

	counter0 = 0;
	counter1 = 0;

	IO_clr_CLRD();	// Borro el latch llevandolo a 0.
	IO_set_CLRD();	// Lo dejo en reposo en 1

}
//------------------------------------------------------------------------------------
ISR(PORTA_INT0_vect)
{
	// Esta ISR se activa cuando el contador D2 (PA2) genera un flaco se subida.
	// Solo avisa a la tarea principal ( que esta dormida ) que se levante y cuente
	// el pulso y haga el debounced.
	// Dado que los ISR de los 2 contadores son los que despiertan a la tarea, debo
	// indicarle de donde proviene el llamado
	wakeup_for_C0 = true;
	vTaskNotifyGiveFromISR( xHandle_tkCounters , &xHigherPriorityTaskWokenDigital );
	PORTA.INTFLAGS = PORT_INT0IF_bm;
}
//------------------------------------------------------------------------------------
ISR(PORTB_INT0_vect)
{
	// Esta ISR se activa cuando el contador D1 (PB2) genera un flaco se subida.
	// Solo avisa a la tarea principal ( que esta dormida ) que se levante y cuente
	// el pulso y haga el debounced.
	// Dado que los ISR de los 2 contadores son los que despiertan a la tarea, debo
	// indicarle de donde proviene el llamado
	wakeup_for_C1 = true;
	vTaskNotifyGiveFromISR( xHandle_tkCounters , &xHigherPriorityTaskWokenDigital );
	PORTB.INTFLAGS = PORT_INT0IF_bm;
}
//------------------------------------------------------------------------------------
// FUNCIONES PUBLICAS
//------------------------------------------------------------------------------------
void pub_counters_read( uint16_t *count0, uint16_t *count1, bool clear_counters_flag )
{
	// Retorna el valor de los contadores y de acuerdo a la flag los pone en 0 o no.

	*count0 = counter0;
	*count1 = counter1;

	if ( clear_counters_flag ) {
		counter0 = 0;
		counter1 = 0;
	}

}
//------------------------------------------------------------------------------------
void pub_counter_config_channel( uint8_t channel,char *s_param0 )
{

	while ( xSemaphoreTake( sem_SYSVars, ( TickType_t ) 5 ) != pdTRUE )
		taskYIELD();

	// s_param1 = TIPO
	// s_param1 = NAME
	// Controlo que los parametros sean correctos.
	// Channel ID
	if ( channel > NRO_COUNTERS_CHANNELS ) goto EXIT;

	// NOMBRE
	pub_control_string(s_param0);
	snprintf_P( systemVars.c_ch_name[channel], PARAMNAME_LENGTH, PSTR("%s\0"), s_param0 );

EXIT:
	xSemaphoreGive( sem_SYSVars );

}
//------------------------------------------------------------------------------------
void pub_counter_load_defaults(void)
{

uint8_t channel;

	for ( channel = 0; channel < NRO_COUNTERS_CHANNELS; channel++) {
		snprintf_P( systemVars.c_ch_name[channel], PARAMNAME_LENGTH, PSTR("C%d\0"),channel );
	}

}
//------------------------------------------------------------------------------------


