/*
 * spxR3_IO8_tkData.c
 *
 *  Created on: 30 oct. 2018
 *      Author: pablo
 */
#include "spxR3_IO8.h"

// Este factor es porque la resistencia shunt es de 7.3 por lo que con 20mA llegamos hasta 3646 y no a 4096
#define FACTOR_CORRECCION_RSHUNT	3646

//------------------------------------------------------------------------------------
// PROTOTIPOS

static bool pv_data_guardar_BD( void );
static void pv_data_signal_to_tkgprs(void);
static void pv_data_read_analog(void);

// VARIABLES LOCALES
static st_data_frame pv_data_frame;

uint8_t wdg_counter_data;

//------------------------------------------------------------------------------------
void tkData(void * pvParameters)
{

( void ) pvParameters;

uint32_t waiting_ticks;
TickType_t xLastWakeTime;

	// Espero la notificacion para arrancar
	while ( !startTask )
		vTaskDelay( ( TickType_t)( 100 / portTICK_RATE_MS ) );

	xprintf_P( PSTR("starting tkData..\r\n\0"));

	// Configuro los INA para promediar en 128 valores.
	INA_config_avg128(0);
	INA_config_avg128(1);
	INA_config_avg128(2);

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    // Al arrancar poleo a los 10s
    waiting_ticks = (uint32_t)(10) * 1000 / portTICK_RATE_MS;

    wdg_counter_data = 3;

	// loop
	for( ;; )
	{
		// El sanity chech pasa por 3 puntos.
		if ( wdg_counter_data == 3 ) {
			pub_ctl_watchdog_kick(WDG_DAT);
			wdg_counter_data = 0;
		} else {
			xprintf_P( PSTR("DATA: WDG ERROR sanity check (%d)\r\n\0"), wdg_counter_data );
		}

		// Espero timerPoll
		vTaskDelayUntil( &xLastWakeTime, waiting_ticks );

		// Leo analog,digital,rtc,salvo en BD e imprimo.
		pub_data_read_frame( true );	// wdg_counter_data = 1

		// Muestro en pantalla.
		pub_data_print_frame( true );	// wdg_counter_data = 2

		// Salvo en BD ( si no es el primer frame )
		if ( pv_data_guardar_BD() ) {	// wdg_counter_data = 3
			// Aviso a tkGPRS ( si estoy en modo continuo )
			pv_data_signal_to_tkgprs();
		}

		// Espero un ciclo
		while ( xSemaphoreTake( sem_SYSVars, ( TickType_t ) 5 ) != pdTRUE )
			taskYIELD();

		waiting_ticks = (uint32_t)(systemVars.timerPoll) * 1000 / portTICK_RATE_MS;
		pub_ctl_reload_timerPoll();

		xSemaphoreGive( sem_SYSVars );

	}

}
//------------------------------------------------------------------------------------
static bool pv_data_guardar_BD(void)
{

	// Solo los salvo en la BD si estoy en modo normal.
	// En otros casos ( service, monitor_frame, etc, no.

FAT_t l_fat;
int8_t bytes_written;
static bool primer_frame = true;

	wdg_counter_data++;
	if ( wdg_counter_data != 3 )
		xprintf_P( PSTR("DATA: WDG guardar_en_BD 3->(%d)\r\n\0"),wdg_counter_data);

	// Para no incorporar el error de los contadores en el primer frame no lo guardo.
	if ( primer_frame ) {
		primer_frame = false;
		return(false);
	}

	// Guardo en BD
	bytes_written = FF_writeRcd( &pv_data_frame, sizeof(st_data_frame) );

	if ( bytes_written == -1 ) {
		// Error de escritura o memoria llena ??
		xprintf_P(PSTR("DATA: WR ERROR (%d)\r\n\0"),FF_errno() );
		// Stats de memoria
		FAT_read(&l_fat);
		xprintf_P( PSTR("DATA: MEM [wr=%d,rd=%d,del=%d]\0"), l_fat.wrPTR,l_fat.rdPTR, l_fat.delPTR );
		return(false);
	}

	return(true);

}
//------------------------------------------------------------------------------------
static void pv_data_signal_to_tkgprs(void)
{
	// Aviso a tkGprs que hay un frame listo. En modo continuo lo va a trasmitir enseguida.
	while ( xTaskNotify(xHandle_tkGprsRx, TK_FRAME_READY , eSetBits ) != pdPASS ) {
		vTaskDelay( ( TickType_t)( 100 / portTICK_RATE_MS ) );
	}
}
//------------------------------------------------------------------------------------
static void pv_data_read_analog(void)
{

	// Lee todos los canales analogicos y los deja en la estructura st_analog_frame.

uint8_t channel;
uint16_t raw_val;
float mag_val;

	// Leo los canales analogicos de datos.
	for ( channel = 0; channel < NRO_ANALOG_CHANNELS; channel++ ) {
		pub_data_read_anChannel (channel, &raw_val, &mag_val );
		pv_data_frame.analog_data[channel] = mag_val;
	}

}
//------------------------------------------------------------------------------------
// FUNCIONES PUBLICAS
//------------------------------------------------------------------------------------
void pub_data_read_frame(bool wdg_control)
{

int8_t xBytes;

	// Funcion usada para leer los datos de todos los modulos, guardarlos en memoria
	// e imprimirlos.
	// La usa por un lado tkData en forma periodica y desde el cmd line cuando se
	// da el comando read frame.

	// Leo los canales analogicos.
	pv_data_read_analog();
	pub_digital_read_inputs( &pv_data_frame.digital_data, true );
	pub_counters_read( &pv_data_frame.counters_data[0], &pv_data_frame.counters_data[1], true );

	// Agrego el timestamp
	xBytes = RTC_read_dtime( &pv_data_frame.rtc);
	if ( xBytes == -1 )
		xprintf_P(PSTR("ERROR: I2C:RTC:pub_data_read_frame\r\n\0"));

	if ( wdg_control ) {
		wdg_counter_data++;
		if ( wdg_counter_data != 1 )
			xprintf_P( PSTR("DATA: WDG read_frame 1->(%d)\r\n\0"),wdg_counter_data);
	}

}
//------------------------------------------------------------------------------------
void pub_data_print_frame(bool wdg_control)
{
	// Imprime el frame actual en consola

uint8_t channel;

	// HEADER
	xprintf_P(PSTR("frame: " ) );
	// timeStamp.
	xprintf_P(PSTR("%04d%02d%02d,"),pv_data_frame.rtc.year,pv_data_frame.rtc.month,pv_data_frame.rtc.day );
	xprintf_P(PSTR("%02d%02d%02d"),pv_data_frame.rtc.hour,pv_data_frame.rtc.min, pv_data_frame.rtc.sec );


	// Valores analogicos
	// Solo muestro los que tengo configurados.
	for ( channel = 0; channel < NRO_ANALOG_CHANNELS; channel++) {
		if ( ! strcmp ( systemVars.an_ch_name[channel], "X" ) )
			continue;

		xprintf_P(PSTR(",%s=%.02f"),systemVars.an_ch_name[channel],pv_data_frame.analog_data[channel] );
	}

	// Valores digitales. Lo que mostramos depende de lo que tenemos configurado
	// Niveles logicos.
	for ( channel = 0; channel < NRO_DIGITAL_CHANNELS; channel++) {
		// Si el canal no esta configurado no lo muestro.
		if ( ! strcmp ( systemVars.d_ch_name[channel], "X" ) )
			continue;

		xprintf_P(PSTR(",%s=%d"),systemVars.d_ch_name[channel],pv_data_frame.digital_data[channel] );
	}

	// Contadores
	for ( channel = 0; channel < NRO_COUNTERS_CHANNELS; channel++) {
		// Si el canal no esta configurado no lo muestro.
		if ( ! strcmp ( systemVars.c_ch_name[channel], "X" ) )
			continue;

		xprintf_P(PSTR(",%s=%d"), systemVars.c_ch_name[channel], pv_data_frame.counters_data[channel] );
	}


	// TAIL
	xprintf_P(PSTR("\r\n\0") );

	if ( wdg_control ) {
		wdg_counter_data++;
		if ( wdg_counter_data != 2 )
			xprintf_P( PSTR("DATA: WDG print_frame 2->(%d)\r\n\0"),wdg_counter_data);
	}
}
//------------------------------------------------------------------------------------
void pub_data_read_anChannel ( uint8_t channel, uint16_t *raw_val, float *mag_val )
{

	// Lee un canal analogico y devuelve en raw_val el valor leido del conversor A/D y en
	// mag_val el valor convertido a la magnitud configurada.
	// Se utiliza desde el modo comando como desde el modulo de poleo de las entradas.

uint16_t an_raw_val;
float an_mag_val;
float I,M,P;
uint16_t D;

	an_raw_val = ACH_read_channel(channel);
	*raw_val = an_raw_val;

	// Convierto el raw_value a la magnitud
	// Calculo la corriente medida en el canal
	I = (float)( an_raw_val) * 20 / ( systemVars.coef_calibracion[channel] + 1);

	// Calculo la magnitud
	P = 0;
	D = systemVars.imax[channel] - systemVars.imin[channel];

	an_mag_val = 0.0;
	if ( D != 0 ) {
		// Pendiente
		P = (float) ( systemVars.mmax[channel]  -  systemVars.mmin[channel] ) / D;
		// Magnitud
		M = (float) (systemVars.mmin[channel] + ( I - systemVars.imin[channel] ) * P);
		an_mag_val = M;

	} else {
		// Error: denominador = 0.
		an_mag_val = -999.0;
	}

	*mag_val = an_mag_val;

}
//------------------------------------------------------------------------------------
void pub_data_config_timerpoll ( char *s_timerpoll )
{
	// Configura el tiempo de poleo.
	// Se utiliza desde el modo comando como desde el modo online
	// El tiempo de poleo debe estar entre 15s y 3600s

	while ( xSemaphoreTake( sem_SYSVars, ( TickType_t ) 5 ) != pdTRUE )
		taskYIELD();

	systemVars.timerPoll = atoi(s_timerpoll);

	if ( systemVars.timerPoll < 15 )
		systemVars.timerPoll = 15;

	if ( systemVars.timerPoll > 3600 )
		systemVars.timerPoll = 300;

	xSemaphoreGive( sem_SYSVars );
	return;
}
//------------------------------------------------------------------------------------
bool pub_data_config_analog_channel( uint8_t channel,char *s_aname,char *s_imin,char *s_imax,char *s_mmin,char *s_mmax )
{

	// Configura los canales analogicos. Es usada tanto desde el modo comando como desde el modo online por gprs.

bool retS = false;

	while ( xSemaphoreTake( sem_SYSVars, ( TickType_t ) 5 ) != pdTRUE )
		taskYIELD();

	pub_control_string(s_aname);

	if ( ( channel >=  0) && ( channel < NRO_ANALOG_CHANNELS) ) {
		snprintf_P( systemVars.an_ch_name[channel], PARAMNAME_LENGTH, PSTR("%s\0"), s_aname );
		if ( s_imin != NULL ) { systemVars.imin[channel] = atoi(s_imin); }
		if ( s_imax != NULL ) { systemVars.imax[channel] = atoi(s_imax); }
		if ( s_mmin != NULL ) { systemVars.mmin[channel] = atoi(s_mmin); }
		if ( s_mmax != NULL ) { systemVars.mmax[channel] = atof(s_mmax); }
	}

	retS = true;

	xSemaphoreGive( sem_SYSVars );
	return(retS);
}
//------------------------------------------------------------------------------------
void pub_data_load_defaults(void)
{

	// Realiza la configuracion por defecto.

uint8_t channel;

	systemVars.timerPoll = 60;

	for ( channel = 0; channel < NRO_ANALOG_CHANNELS; channel++) {
		systemVars.coef_calibracion[channel] = 3646;
		systemVars.imin[channel] = 0;
		systemVars.imax[channel] = 20;
		systemVars.mmin[channel] = 0;
		systemVars.mmax[channel] = 6.0;
//		systemVars.a_ch_modo[channel] = 'L';	// Modo local
		snprintf_P( systemVars.an_ch_name[channel], PARAMNAME_LENGTH, PSTR("A%d\0"),channel );
	}
}
//------------------------------------------------------------------------------------

