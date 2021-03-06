/*
 * l_iopines.c
 *
 *  Created on: 29 oct. 2018
 *      Author: pablo
 */
#include "l_iopines.h"
#include "l_mcp23018.h"
#include "l_bytes.h"

//------------------------------------------------------------------------------------
// ENTRADAS DIGITALES DEL MODEM
//------------------------------------------------------------------------------------
uint8_t IO_read_DCD(void)
{
	return( PORT_GetBitValue(&GPRS_DCD_PORT, GPRS_DCD_BITPOS));
}
//------------------------------------------------------------------------------------
uint8_t IO_read_RI(void)
{
	return( PORT_GetBitValue(&GPRS_RI_PORT, GPRS_RI_BITPOS));
}
//------------------------------------------------------------------------------------
uint8_t IO_read_RTS(void)
{
	return( PORT_GetBitValue(&GPRS_RTS_PORT, GPRS_RTS_BITPOS));
}
//------------------------------------------------------------------------------------
// ENTRADAS DIGITALES DE CONTEO DE PULSOS Y NIVELES
//------------------------------------------------------------------------------------
uint8_t IO_read_PB2(void)
{
	return( PORT_GetBitValue(&PB2_PORT, PB2_BITPOS));
}
//------------------------------------------------------------------------------------
uint8_t IO_read_PA2(void)
{
	return( PORT_GetBitValue(&PA2_PORT, PA2_BITPOS));
}
//------------------------------------------------------------------------------------
uint8_t IO_read_TERMCTL_PIN(void)
{
	return( PORT_GetBitValue(&TERMCTL_PIN_PORT, TERMCTL_PIN_BITPOS));
}
//------------------------------------------------------------------------------------
uint8_t IO_read_BAUD_PIN(void)
{
	return( PORT_GetBitValue(&BAUD_PIN_PORT, BAUD_PIN_BITPOS));
}
//------------------------------------------------------------------------------------
void IO_config_PB2(void)
{
	// ( PB2 ) se utiliza para generar una interrupcion por flanco por lo tanto
	// hay que configurarlo.

	PORT_SetPinAsInput( &PB2_PORT, PB2_BITPOS);
	PB2_PORT.PIN2CTRL = 0x01;	// sense rising edge
	PB2_PORT.INTCTRL = 0x01;	// Dispara la interrupcion 0.
	PB2_PORT.INT0MASK = 0x04;	// Asocio el pin 2 a dicha interrupcion
}
//------------------------------------------------------------------------------------
void IO_config_PA2(void)
{
	// ( PA2 ) se utiliza para generar una interrupcion por flanco por lo tanto
	// hay que configurarlo.

	PORT_SetPinAsInput( &PA2_PORT, PA2_BITPOS);
	PA2_PORT.PIN2CTRL = 0x01;	// sense rising edge
	PA2_PORT.INTCTRL = 0x01;	// Dispara la interrupcion 0 con level 1
	PA2_PORT.INT0MASK = 0x04;	// Asocio el pin 2 a dicha interrupcion


}
//------------------------------------------------------------------------------------
/*
 *  This function configures interrupt 1 to be associated with a set of pins and
 *  sets the desired interrupt level.
 *
 *  port       The port to configure.
 *  intLevel   The desired interrupt level for port interrupt 1.
 *  pinMask    A mask that selects the pins to associate with port interrupt 1.
 */
/*
void PORT_ConfigureInterrupt0( PORT_t * port,PORT_INT0LVL_t intLevel,uint8_t pinMask )
{
	port->INTCTRL = ( port->INTCTRL & ~PORT_INT0LVL_gm ) | intLevel;
	port->INT0MASK = pinMask;
}

//------------------------------------------------------------------------------------
void PORT_ConfigureInterrupt1( PORT_t * port, PORT_INT1LVL_t intLevel,uint8_t pinMask )
{
	port->INTCTRL = ( port->INTCTRL & ~PORT_INT1LVL_gm ) | intLevel;
	port->INT1MASK = pinMask;
}
*/
//------------------------------------------------------------------------------------
int8_t IO_read_DIN( uint8_t pin)
{

	// Los pines pueden ser 0 o 1. Cualquier otro valor es error

uint8_t data;
int8_t rdBytes;

	rdBytes = MCP_read( MCP_GPIOA, (char *)&data, 1 );

	if ( rdBytes == -1 ) {
		xprintf_P(PSTR("ERROR: IO_read_DIN\r\n\0"));
		return(-1);
	}

	data = (data & ( 1 << pin )) >> pin;

	return(data);


}
//------------------------------------------------------------------------------------
int8_t IO_set_DOUT(uint8_t pin)
{
	// Leo el MCP, aplico la mascara y lo escribo de nuevo

uint8_t data;
int8_t xBytes;

	// Control de entrada valida
	if ( pin > 7 ) {
		xprintf_P(PSTR("ERROR: IO_set_DOUT (pin<7)!!!\r\n\0"));
		return(-1);
	}

	xBytes = MCP_read( MCP_GPIOB, (char *)&data, 1 );
	if ( xBytes == -1 ) {
		xprintf_P(PSTR("ERROR: IO_set_DOUT(1)\r\n\0"));
		return(-1);
	}

	// Aplico la mascara para setear el pin dado
	// En el MCP estan en orden inverso
	data |= ( 1 << ( 7 - pin )  );

	xBytes = MCP_write(MCP_OLATB, (char *)&data, 1 );
	if ( xBytes == -1 ) {
		xprintf_P(PSTR("ERROR: IO_set_DOUT(2)\r\n\0"));
		return(-1);
	}

	return(1);

}
//------------------------------------------------------------------------------------
int8_t IO_clr_DOUT(uint8_t pin)
{
	// Leo el MCP, aplico la mascara y lo escribo de nuevo

uint8_t data;
int8_t xBytes;

	// Control de entrada valida
	if ( pin > 7 ) {
		xprintf_P(PSTR("ERROR: IO_clr_DOUT (pin<7)!!!\r\n\0"));
		return(-1);
	}

	xBytes = MCP_read( MCP_GPIOB, (char *)&data, 1 );
	if ( xBytes == -1 ) {
		xprintf_P(PSTR("ERROR: IO_clr_DOUT(1)\r\n\0"));
		return(-1);
	}

	// Aplico la mascara para setear el pin dado
	data &= ~( 1 << ( 7 - pin ) );

	xBytes = MCP_write(MCP_OLATB, (char *)&data, 1 );
	if ( xBytes == -1 ) {
		xprintf_P(PSTR("ERROR: IO_clr_DOUT(2)\r\n\0"));
		return(-1);
	}

	return(1);
}
//------------------------------------------------------------------------------------
int8_t IO_reflect_DOUTPUTS(uint8_t output_value )
{
uint8_t data;
int8_t xBytes;

	// Escribe todas las salidas a la vez.
	// En el hardware las salidas son inversas a los bits ( posiciones )

	data = twiddle_bits( output_value );

//	xprintf_P(PSTR("IO: %d 0x%0x, DAT=0x%0x\r\n\0"),output_value,output_value, data);
	xBytes = MCP_write(MCP_OLATB, (char *)&data, 1 );
	if ( xBytes == -1 ) {
		xprintf_P(PSTR("ERROR: IO_reflect_DOUTPUTS\r\n\0"));
		return(-1);
	}

	return(1);
}
//------------------------------------------------------------------------------------


