/*
 * sp5KV5_tkGprs_esperar_apagado.c
 *
 *  Created on: 28 de abr. de 2017
 *      Author: pablo
 *
 *  En este subestado el modem se apaga y espera aqui hasta que expire el tiempo
 *  del timerdial.
 *  Si esta en modo continuo, (timerdial = 0 ), espero 60s.
 *  En este estado es donde entro en el modo tickless !!!.
 */

#include "spxR3_IO8_tkGprs.h"

static int32_t waiting_time;

//------------------------------------------------------------------------------------
bool st_gprs_esperar_apagado(void)
{
	// Calculo el tiempo a esperar y espero. El intervalo no va a considerar el tiempo
	// posterior de proceso.

bool exit_flag = false;

	GPRS_stateVars.state = G_ESPERA_APAGADO;

	// Secuencia para apagar el modem y dejarlo en modo low power.
	xprintf_P( PSTR("GPRS: modem off.\r\n\0"));

	// Actualizo todas las variables para el estado apagado.
	GPRS_stateVars.modem_prendido = false;
	strncpy_P(systemVars.dlg_ip_address, PSTR("000.000.000.000\0"),16);
	systemVars.csq = 0;
	systemVars.dbm = 0;
	GPRS_stateVars.signal_redial = false;
	GPRS_stateVars.dcd = 1;

	// Apago
	pub_gprs_modem_pwr_off();

	waiting_time = 60;

	// Espera
	while ( ! exit_flag )  {

		// Reinicio el watchdog
		pub_ctl_watchdog_kick(WDG_GPRSTX);

		// Espero de a 5s para poder entrar en tickless.
		vTaskDelay( (portTickType)( 5000 / portTICK_RATE_MS ) );
		waiting_time -= 5;

		// Expiro el tiempo de espera. Veo si estoy dentro del intervalo de
		// pwrSave y entonces espero de a 10 minutos mas.
		if ( waiting_time <= 0 ) {
			// Salgo con true.
			exit_flag = bool_CONTINUAR;
			goto EXIT;
		}

	}

EXIT:

	// No espero mas y salgo del estado prender.
	GPRS_stateVars.signal_redial = false;
	waiting_time = -1;
	return(exit_flag);

}
//------------------------------------------------------------------------------------
