/*
 * main.c
 *
 *  Created on: 18 de oct. de 2016
 *      Author: pablo
 *
 *  avrdude -v -Pusb -c avrispmkII -p x256a3 -F -e -U flash:w:spx.hex
 *  avrdude -v -Pusb -c avrispmkII -p x256a3 -F -e
 *
 *  REFERENCIA: /usr/lib/avr/include/avr/iox256a3b.h
 *
 *  El watchdog debe estar siempre prendido por fuses.
 *  FF 0A FD __ F5 D6
 *
 *  PROGRAMACION FUSES:
 *  /usr/bin/avrdude -px256a3b -cavrispmkII -Pusb -u -Uflash:w:spx.hex:a -Ufuse0:w:0xff:m -Ufuse1:w:0x0:m -Ufuse2:w:0xff:m -Ufuse4:w:0xff:m -Ufuse5:w:0xff:m
 *  /usr/bin/avrdude -px256a3b -cavrispmkII -Pusb -u -Ufuse0:w:0xff:m -Ufuse1:w:0x0:m -Ufuse2:w:0xff:m -Ufuse4:w:0xff:m -Ufuse5:w:0xff:m
 *
 *  Para ver el uso de memoria usamos
 *  avr-nm -n spxR1.elf | more
 *
 *------------------------------------------------------------------------------------------
 * Version 0.1.5 @ 20190311
 * - Agrego 3 defines ( UTE / SPY / OSE ).
 *   Se aplican en GPRS ( defaults ), Cmd ( indica la compilacion ), tkDigital.
 * - Agrego que cada vez que cambio las entradas digitales, reinicio el MCP ( MCP_init )
 *   ya que hay veces que queda con algún registro mal por ruidos externos.
 * -
 *------------------------------------------------------------------------------------------
 * Version 0.0.5 @ 2018-11-26
 * Solo en la rama UTE, los canales digitales pueden medir nivel o tiempo
 * D0..D3 miden solo nivel y D4..D7 miden solo tiempo.
 * En modo SPX los canales digitales solo miden nivel logico.
 *
 *------------------------------------------------------------------------------------------
 * Version 0.0.2 @ 2018-11-19:
 * Genero 2 ramas: UTE y SPX
 * UTE: No se configura por INITS. El init es limitado para solo obtener el clock.
 *      No maneja salidas.
 *
 * SPX: Manda INIT y se reconfigura remotamente
 *      Maneja las salidas con el FRAME "STATUS".
 *
 * En UTE dividimos las entradas digitales en 2: aquellas que solo miden nivel logico (0..3)
 * y las que miden el tiempo que estan en 1 ( 4..7
 * En modo SPX solo mido el nivel logico
 *
 *------------------------------------------------------------------------------------------
 * Version 0.0.1 @ 2018-10-30:
 *
 * Version inicial basada en spxR3.
 * Elimino todo excepto tkCmd y tkCtl.
 * No tiene sentido usar el modo TICKLESS pero para no desarmar todo, en la tkCmd
 * elimino el chequeo del TERM_PIN.
 * Como el tiempo en idle para entrar en tickless es alto, nunca entro en ese modo.
 * Problema: Como recuperar el bus I2C si falla !!!
 * Problema: La terminal se cuelga ( pero el led continua )
 *
 * En el modo cmd solo dejo las funciones relativas a la placa logica.
 * Creo la libreria para manejar el MCP23018 y las funciones de cmdMode.
 * Agrego las funciones de libreria para leer las entradas digitales y en modo
 * comando agrego "read din X"
 * Programo la lectura de los contadores.
 * Agrego las librerias para leer y configurar los INA.
 * Agrego la tarea de lectura de datos ( tkData ).
 * Agrego las funciones de libreria para escribir las salidas digitales.
 * Agrego tkGprs.
 * En esta version solo trabaja en modo continuo y no hay pwrSave,tdial ni redial ni proceso
 * señales.
 * Las salidas no se configuran en el INIT.
 *
 * Agrego el manejo de las salidas.
 * Las salidas son un byte del systemVars que si cambia, la tarea de ouputs actualiza sus salidas.
 *
 * Las entradas digitales las configuro para medir nivel o para medir el tiempo
 * que estan arriba, en ticks de 100ms.
 * Este modo se configura de tkCMD
 *
 * Leo el RST.STATUS al arrancar y determino la causa del reset que la trasmito en el proximo INIT.
 *
 * Comunicaciones:
 * 1- No se configura nada en forma remota
 * 2- Los canales de datos que se trasmiten son solo los configurados ( nombre != X ).
 */

// PENDIENTE:
// Memoria roll-over
// Lento al SAVE

#include "spxR3_IO8.h"

//----------------------------------------------------------------------------------------
// XMEGA:
// http://microfluidics.utoronto.ca/gitlab/dstat/dstat-firmware/raw/1.01/DSTAT/src/asf/xmega/drivers/cpu/xmega_reset_cause.h


// http://www.atmel.com/webdoc/AVRLibcReferenceManual/FAQ_1faq_softreset.html
// http://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html
//
// Function Pototype
//uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
//void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

// Function Implementation
/*
void wdt_init(void)
{
    // Como los fusibles estan para que el WDG siempre este prendido, lo reconfiguro a 8s lo
    // antes posible
	WDT_EnableAndSetTimeout(  WDT_PER_8KCLK_gc );

    return;
}
*/
//------------------------------------------------------------------------------------
int main( void )
{
	wdg_resetCause = RST.STATUS;
	RST.STATUS = wdg_resetCause;
	//RST_PORF_bm | RST_EXTRF_bm | RST_BORF_bm | RST_WDRF_bm | RST_PDIRF_bm | RST_SRF_bm | RST_SDRF_bm;

	// Clock principal del sistema
	u_configure_systemMainClock();
	u_configure_RTC32();

	// Configuramos y habilitamos el watchdog a 8s.
	WDT_EnableAndSetTimeout(  WDT_PER_8KCLK_gc );
	if ( WDT_IsWindowModeEnabled() )
		WDT_DisableWindowMode();

	set_sleep_mode(SLEEP_MODE_PWR_SAVE);

	initMCU();

	frtos_open(fdGPRS, 115200);
	frtos_open(fdI2C, 100 );

	//  Configuro el puerto serial de la terminal
//	if ( IO_read_BAUD_PIN() == 0 ) {
		// USB
		frtos_open(fdTERM, 115200 );
//	} else {
		// BLUETOOTH
//		frtos_open(fdTERM, 9600 );
//	}

	// Creo los semaforos
	sem_SYSVars = xSemaphoreCreateMutexStatic( &SYSVARS_xMutexBuffer );
	xprintf_init();
	FAT_init();

	startTask = false;

	xTaskCreate(tkCtl, "CTL", tkCtl_STACK_SIZE, NULL, tkCtl_TASK_PRIORITY,  &xHandle_tkCtl );
	xTaskCreate(tkCmd, "CMD", tkCmd_STACK_SIZE, NULL, tkCmd_TASK_PRIORITY,  &xHandle_tkCmd);
	xTaskCreate(tkCounters, "CNT", tkCounters_STACK_SIZE, NULL, tkCounters_TASK_PRIORITY,  &xHandle_tkCounters);
	xTaskCreate(tkData, "DAT", tkData_STACK_SIZE, NULL, tkData_TASK_PRIORITY,  &xHandle_tkData);
	xTaskCreate(tkDigital, "DIGI", tkDigital_STACK_SIZE, NULL, tkDigital_TASK_PRIORITY,  &xHandle_tkDigital);
	xTaskCreate(tkGprsRx, "RX", tkGprs_rx_STACK_SIZE, NULL, tkGprs_rx_TASK_PRIORITY,  &xHandle_tkGprsRx );
	xTaskCreate(tkGprsTx, "TX", tkGprs_tx_STACK_SIZE, NULL, tkGprs_tx_TASK_PRIORITY,  &xHandle_tkGprsTx );
	xTaskCreate(tkOutputs, "DOUT", tkOutputs_STACK_SIZE, NULL, tkOutputs_TASK_PRIORITY,  &xHandle_tkOutputs );

	/* Arranco el RTOS. */
	vTaskStartScheduler();

	// En caso de panico, aqui terminamos.
	exit (1);

}
//-----------------------------------------------------------
void vApplicationIdleHook( void )
{
	// Como trabajo en modo tickless no entro mas en modo sleep aqui.
//	if ( sleepFlag == true ) {
//		sleep_mode();
//	}
}

//-----------------------------------------------------------
/* Define the function that is called by portSUPPRESS_TICKS_AND_SLEEP(). */
//------------------------------------------------------------------------------------
void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName )
{
	// Es invocada si en algun context switch se detecta un stack corrompido !!
	// Cuando el sistema este estable la removemos.
	// En FreeRTOSConfig.h debemos habilitar
	// #define configCHECK_FOR_STACK_OVERFLOW          2

	xprintf_P( PSTR("PANIC:%s !!\r\n\0"),pcTaskName);

}
//------------------------------------------------------------------------------------

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
//------------------------------------------------------------------------------------

