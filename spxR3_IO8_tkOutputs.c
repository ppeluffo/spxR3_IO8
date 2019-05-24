/*
 * spxR3_IO8_tkOutputs.c
 *
 *  Created on: 6 nov. 2018
 *      Author: pablo
 */


#include "spxR3_IO8.h"

static void pv_tkOutputs_init(void);
void pv_set_hwoutputs(void);

uint8_t o_control;
static uint16_t o_timer, o_timer_boya, o_timer_sistema;
#define TIMEOUT_O_TIMER			600
#define TIMEOUT_O_TIMER_BOYA	 60

#define RELOAD_TIMER()	( o_timer = TIMEOUT_O_TIMER )
#define RELOAD_TIMER_BOYA()	( o_timer_boya = TIMEOUT_O_TIMER_BOYA )
#define STOP_TIMER_BOYA()	( o_timer_boya = 0 )
//------------------------------------------------------------------------------------
void tkOutputs(void * pvParameters)
{

( void ) pvParameters;

	// Espero la notificacion para arrancar
	while ( !startTask )
		vTaskDelay( ( TickType_t)( 100 / portTICK_RATE_MS ) );

	pv_tkOutputs_init();

	// Control de las salidas
	o_control = CTL_BOYA;
	o_timer = 0;
	o_timer_sistema = 15;

	xprintf_P( PSTR("starting tkOutputs..\r\n\0"));

	// loop
	for( ;; )
	{

		pub_ctl_watchdog_kick(WDG_OUT);

		vTaskDelay( ( TickType_t)( 1000 / portTICK_RATE_MS ) );

		// Monitoreo quien controla las salidas: BOYA o SISTEMA
		// 1) Si las salidas estan en BOYA y el valor pasa a ser != 0x00 el control pasa
		//    al SISTEMA y arranca el timer.
		// 2) Si el control es del SISTEMA y el timer expiro, paso el control a las BOYAS

		switch ( o_control ) {
		case CTL_BOYA:
			// 1) Si las salidas estan en BOYA y el valor de las salidas pasa a ser != 0x00 el control pasa
			//    al SISTEMA y arranca el timer.
			//	  Solo salgo del control xBoya cuando el GPRS recibio datos de las salidas.
			if ( systemVars.d_outputs != 0x00 ) {
				o_control = CTL_SISTEMA;	// Paso el control al sistema
				pv_set_hwoutputs();			// Seteo las salidas HW.
				o_timer = TIMEOUT_O_TIMER;	// Arranco el timer
				xprintf_P( PSTR("OUTPUT CTL to SISTEMA !!!. (Reinit outputs timer)\r\n\0"));
				STOP_TIMER_BOYA();			// Paro el timer de la boya
			}
			break;

		case CTL_SISTEMA:

			if ( o_timer == 0 ) {
				// El control es del SISTEMA y el timer expiro: paso el control a las BOYAS
				systemVars.d_outputs = 0x00;	// Apago las salidas
				o_control = CTL_BOYA;			// Paso el control a las boyas.
				pv_set_hwoutputs();				// Seteo las salidas HW.( 0x00 )
				xprintf_P( PSTR("OUTPUT CTL to BOYAS !!!. (set outputs to 0x00)\r\n\0"));

				// Arranco el timer de las boyas
				RELOAD_TIMER_BOYA();
			}

			// Cuando el modem esta prendido no hago nada ya que las salidas las maneja el server
			if ( pub_modem_prendido() ) {
				o_timer_sistema = 15;		// reseteo el timer
			} else {
				// Con el modem apagado.
				if ( o_timer_sistema-- == 0 ) {		// disminuyo el timer
					o_timer_sistema = 15;
					pv_set_hwoutputs();
					xprintf_P( PSTR("OUTPUT reload: Sistema con modem apagado)\r\n\0"));
				}
			}
			break;

		default:
			xprintf_P( PSTR("ERROR Control outputs: Pasa a BOYA !!\r\n\0"));
			o_control = CTL_BOYA;
			break;
		}

		// Corro el timer
		if ( o_timer > 0) {
			// Estoy en control por SISTEMA: voy contando la antiguedad del dato.
			o_timer--;
		}

		if ( o_timer_boya > 0 ) {
			o_timer_boya--;
			if ( o_timer_boya == 0 ) {
				RELOAD_TIMER_BOYA();
				systemVars.d_outputs = 0x00;
				pv_set_hwoutputs();
				xprintf_P( PSTR("MODO BOYA: reload timer boya\r\n\0"));
			}
		}
	}

}
//------------------------------------------------------------------------------------
static void pv_tkOutputs_init(void)
{

	// AL iniciar el sistema debo leer el valor que tiene el MCP.OLATB
	// y activar las salidas con este valor

int xBytes = 0;
uint8_t data;

	//outputs = systemVars.d_outputs;
	xBytes = MCP_read( 21, (char *)&data, 1 );
	if ( xBytes == -1 ) {
		xprintf_P(PSTR("OUTPUTS INIT ERROR: I2C:MCP:pv_cmd_rwMCP\r\n\0"));
		data = 0x00;
	}

	if ( xBytes > 0 ) {
		xprintf_P( PSTR( "OUTPUTS INIT OK: VALUE=0x%x\r\n\0"),data);
	}

	// Como el dato esta espejado lo debo girar
	data = twiddle_bits(data);
	systemVars.d_outputs = data;
	// Fijo las salidas en el mismo valor que tenia el OLATB
	pv_set_hwoutputs();

}
//------------------------------------------------------------------------------------
void pv_set_hwoutputs(void)
{
	// Pone el valor de systemVars.d_outputs en los pines de la salida

int8_t ret_code;

	ret_code = IO_reflect_DOUTPUTS(systemVars.d_outputs);
	if ( ret_code == -1 ) {
		xprintf_P( PSTR("I2C ERROR pv_set_hwoutputs\r\n\0"));
	}
	xprintf_P( PSTR("tkOutputs: SET [0x%02x][%c%c%c%c%c%c%c%c]\r\n\0"), systemVars.d_outputs,  BYTE_TO_BINARY( systemVars.d_outputs ) );
}
//------------------------------------------------------------------------------------
void pub_output_load_defaults(void)
{
	systemVars.d_outputs = 0x00;
}
//------------------------------------------------------------------------------------
void pub_output_set( uint8_t dout, bool force )
{
	// Funcion para setear el valor de las salidas desde el resto del programa.
	// La usamos desde tkGprs cuando en un frame nos indican cambiar las salidas.
	// Como el cambio depende de quien tiene el control y del timer, aqui vemos si
	// se cambia o se ignora.

	// Guardo el valor recibido
	systemVars.d_outputs = dout;

	// En caso de que venga del modo comando, forzamos el setear el HW.
	if ( force == true ) {
		pv_set_hwoutputs();
	} else {
		// Si viene del gprs
		// Solo lo reflejo en el HW si el control lo tiene el sistema
		if ( o_control == CTL_SISTEMA ) {
			pv_set_hwoutputs();
			RELOAD_TIMER();
		}
	}

}
//------------------------------------------------------------------------------------
uint16_t pub_output_read_datatimer(void)
{
	// Devuelve el valor del timer. Se usa en tkCMD.status

	return(o_timer);
}
//------------------------------------------------------------------------------------
outputs_control_t pub_output_read_control(void)
{
	// Devuelve quien tiene el control de las salidas: BOYA o SISTEMA
	// Se usa en tkCMD.statius

	return(o_control);
}
//------------------------------------------------------------------------------------
void pub_output_mcp_raise_error(void)
{
	// Funcion invocada al leer un PIN ( c/100ms) cuando hubo un error en el
	// bus I2C.
	// Esto cambia la configuracion del MCP por lo que deberia reconfigurarlo.

	xprintf_P( PSTR("I2C BUS ERROR: Reconfiguro MCP !!!\r\n\0"));

	vTaskDelay( ( TickType_t)( 2000 / portTICK_RATE_MS ) );	// Espero 2s que se vaya el ruido
	MCP_init();				// Reconfiguro el MCP
	pv_set_hwoutputs();		// Reconfiguro las salidas del OLATB

}
//------------------------------------------------------------------------------------
