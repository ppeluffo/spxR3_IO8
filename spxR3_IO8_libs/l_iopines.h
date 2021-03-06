/*
 * l_iopines.h
 *
 *  Created on: 29 oct. 2018
 *      Author: pablo
 */

#ifndef SRC_SPXR3_IO8_LIBS_L_IOPINES_H_
#define SRC_SPXR3_IO8_LIBS_L_IOPINES_H_

#include <avr/io.h>
#include <stdbool.h>


#define PORT_SetPinAsOutput( _port, _bitPosition ) ( (_port)->DIR = (_port)->DIR | (1 << _bitPosition) )
#define PORT_SetPinAsInput( _port, _bitPosition ) ( (_port)->DIR = (_port)->DIR & ~(1 << _bitPosition) )

#define PORT_SetOutputBit( _port, _bitPosition ) ( (_port)->OUT = (_port)->OUT | (1 << _bitPosition) )
#define PORT_ClearOutputBit( _port, _bitPosition ) ( (_port)->OUT = (_port)->OUT & ~(1 << _bitPosition) )

#define PORT_GetBitValue( _port , _bitPosition ) ( (((_port)->IN) >> _bitPosition ) & 0x01 )

#define PORT_TogglePins( _port, _toggleMask ) ( (_port)->OUTTGL = _toggleMask )

void PORT_ConfigureInterrupt0( PORT_t * port,PORT_INT0LVL_t intLevel,uint8_t pinMask);
void PORT_ConfigureInterrupt1( PORT_t * port,PORT_INT1LVL_t intLevel, uint8_t pinMask);
//------------------------------------------------------------------------------------
// Control de Frecuencia del TICK
//#define TICK_BITPOS		2
//#define TICK_PORT		PORTA

//#define IO_config_TICK()	PORT_SetPinAsOutput( &TICK_PORT, TICK_BITPOS)
//#define IO_set_TICK()		PORT_SetOutputBit( &TICK_PORT, TICK_BITPOS)
//#define IO_clr_TICK()		PORT_ClearOutputBit( &TICK_PORT, TICK_BITPOS)
//------------------------------------------------------------------------------------
// TWI PE0(SDA)/PE1(SCL)
#define SCL_BITPOS		1
#define SCL_PORT		PORTE

#define IO_config_SCL()		PORT_SetPinAsOutput( &SCL_PORT, SCL_BITPOS)
#define IO_set_SCL()		PORT_SetOutputBit( &SCL_PORT, SCL_BITPOS)
#define IO_clr_SCL()		PORT_ClearOutputBit( &SCL_PORT, SCL_BITPOS)

//------------------------------------------------------------------------------------
// LEDS

#define LED_KA_BITPOS	7
#define LED_KA_PORT		PORTF

#define IO_config_LED_KA()	PORT_SetPinAsOutput( &LED_KA_PORT, LED_KA_BITPOS)
#define IO_set_LED_KA()		PORT_SetOutputBit( &LED_KA_PORT, LED_KA_BITPOS)
#define IO_clr_LED_KA()		PORT_ClearOutputBit( &LED_KA_PORT, LED_KA_BITPOS)

#define LED_COMMS_BITPOS	1
#define LED_COMMS_PORT		PORTF

#define IO_config_LED_COMMS()	PORT_SetPinAsOutput( &LED_COMMS_PORT, LED_COMMS_BITPOS)
#define IO_set_LED_COMMS()		PORT_SetOutputBit( &LED_COMMS_PORT, LED_COMMS_BITPOS)
#define IO_clr_LED_COMMS()		PORT_ClearOutputBit( &LED_COMMS_PORT, LED_COMMS_BITPOS)

//------------------------------------------------------------------------------------
// ANALOG_IN: SENSOR VCC CONTROL

#define SENS_12V_CTL_BITPOS	1
#define SENS_12V_CTL_PORT	PORTA

#define IO_config_SENS_12V_CTL()	PORT_SetPinAsOutput( &SENS_12V_CTL_PORT, SENS_12V_CTL_BITPOS)
#define IO_set_SENS_12V_CTL()		PORT_SetOutputBit( &SENS_12V_CTL_PORT, SENS_12V_CTL_BITPOS)
#define IO_clr_SENS_12V_CTL()		PORT_ClearOutputBit( &SENS_12V_CTL_PORT, SENS_12V_CTL_BITPOS)

//------------------------------------------------------------------------------------
// TERMINAL CONTROL PIN

#define TERMCTL_PIN_BITPOS		4
#define TERMCTL_PIN_PORT		PORTF

#define IO_config_TERMCTL_PIN()		PORT_SetPinAsInput( &TERMCTL_PIN_PORT, TERMCTL_PIN_BITPOS)
uint8_t IO_read_TERMCTL_PIN(void);
//------------------------------------------------------------------------------------
// BAUD RATE SELECTOR

#define BAUD_PIN_BITPOS		6
#define BAUD_PIN_PORT		PORTF

#define IO_config_BAUD_PIN()		PORT_SetPinAsInput( &BAUD_PIN_PORT, BAUD_PIN_BITPOS)
uint8_t IO_read_BAUD_PIN(void);

//------------------------------------------------------------------------------------
// INTERFASE DIGITAL

#define CLR_D_BITPOS	3
#define CLR_D_PORT		PORTB

// contadores(interrupciones)
#define PB2_BITPOS		2
#define PB2_PORT		PORTB
#define PB2_PINMASK		0x02

#define PA2_BITPOS		2
#define PA2_PORT		PORTA
#define PA2_PINMASK		0x04

// Las entradas de pulsos no solo se configuran para pulso sino tambien
// para generar interrupciones.
void IO_config_PB2(void);
void IO_config_PA2(void);

#define IO_config_CLRD()	PORT_SetPinAsOutput( &CLR_D_PORT, CLR_D_BITPOS)
#define IO_set_CLRD()		PORT_SetOutputBit( &CLR_D_PORT, CLR_D_BITPOS)
#define IO_clr_CLRD()		PORT_ClearOutputBit( &CLR_D_PORT, CLR_D_BITPOS)

uint8_t IO_read_PB7(void);
uint8_t IO_read_PB2(void);
//------------------------------------------------------------------------------------
// GPRS

// Salidas de micro, entradas del SIM900
#define GPRS_PWR_BITPOS			4
#define GPRS_PWR_PORT			PORTD
#define IO_config_GPRS_PWR()	PORT_SetPinAsOutput( &GPRS_PWR_PORT, GPRS_PWR_BITPOS)
#define IO_set_GPRS_PWR()		PORT_SetOutputBit( &GPRS_PWR_PORT, GPRS_PWR_BITPOS)
#define IO_clr_GPRS_PWR()		PORT_ClearOutputBit( &GPRS_PWR_PORT, GPRS_PWR_BITPOS)

#define GPRS_SW_BITPOS			5
#define GPRS_SW_PORT			PORTD
#define IO_config_GPRS_SW()		PORT_SetPinAsOutput( &GPRS_SW_PORT, GPRS_SW_BITPOS)
#define IO_set_GPRS_SW()		PORT_SetOutputBit( &GPRS_SW_PORT, GPRS_SW_BITPOS)
#define IO_clr_GPRS_SW()		PORT_ClearOutputBit( &GPRS_SW_PORT, GPRS_SW_BITPOS)

#define GPRS_CTS_BITPOS			4
#define GPRS_CTS_PORT			PORTE
#define IO_config_GPRS_CTS()	PORT_SetPinAsOutput( &GPRS_CTS_PORT, GPRS_CTS_BITPOS)
#define IO_set_GPRS_CTS()		PORT_SetOutputBit( &GPRS_CTS_PORT, GPRS_CTS_BITPOS)
#define IO_clr_GPRS_CTS()		PORT_ClearOutputBit( &GPRS_CTS_PORT, GPRS_CTS_BITPOS)

// Corresponde a ATXMEGA RXD (Input)
#define GPRS_TX_BITPOS			2
#define GPRS_TX_PORT			PORTE
#define IO_config_GPRS_TX()		PORT_SetPinAsInput( &GPRS_TX_PORT, GPRS_TX_BITPOS)

// Entradas al micro, salidas del SIM900
#define GPRS_RTS_BITPOS			5
#define GPRS_RTS_PORT			PORTE
#define IO_config_GPRS_RTS()	PORT_SetPinAsInput( &GPRS_RTS_PORT, GPRS_RTS_BITPOS)
uint8_t IO_read_RTS(void);

#define GPRS_DCD_BITPOS			6
#define GPRS_DCD_PORT			PORTD
#define IO_config_GPRS_DCD()	PORT_SetPinAsInput( &GPRS_DCD_PORT, GPRS_DCD_BITPOS)
uint8_t IO_read_DCD(void);

#define GPRS_RI_BITPOS			7
#define GPRS_RI_PORT			PORTD
#define IO_config_GPRS_RI()		PORT_SetPinAsInput( &GPRS_RI_PORT, GPRS_RI_BITPOS)
uint8_t IO_read_RI(void);

// Corresponde a ATXMEGA TXD (Output)
#define GPRS_RX_BITPOS			3
#define GPRS_RX_PORT			PORTE
#define IO_config_GPRS_RX()		PORT_SetPinAsOutput( &GPRS_RX_PORT, GPRS_RX_BITPOS)

//------------------------------------------------------------------------------------
// XBEE
#define XBEE_PWR_BITPOS			1
#define XBEE_PWR_PORT			PORTD
#define IO_config_XBEE_PWR()	PORT_SetPinAsOutput( &XBEE_PWR_PORT, XBEE_PWR_BITPOS)
#define IO_set_XBEE_PWR()		PORT_SetOutputBit( &XBEE_PWR_PORT, XBEE_PWR_BITPOS)
#define IO_clr_XBEE_PWR()		PORT_ClearOutputBit( &XBEE_PWR_PORT, XBEE_PWR_BITPOS)

#define XBEE_SLEEP_BITPOS		0
#define XBEE_SLEEP_PORT			PORTD
#define IO_config_XBEE_SLEEP()	PORT_SetPinAsOutput( &XBEE_SLEEP_PORT, XBEE_SLEEP_BITPOS)
#define IO_set_XBEE_SLEEP()		PORT_SetOutputBit( &XBEE_SLEEP_PORT, XBEE_SLEEP_BITPOS)
#define IO_clr_XBEE_SLEEP()		PORT_ClearOutputBit( &XBEE_SLEEP_PORT, XBEE_SLEEP_BITPOS)

#define XBEE_RESET_BITPOS		4
#define XBEE_RESET_PORT			PORTC
#define IO_config_XBEE_RESET()	PORT_SetPinAsOutput( &XBEE_RESET_PORT, XBEE_RESET_BITPOS)
#define IO_set_XBEE_RESET()		PORT_SetOutputBit( &XBEE_RESET_PORT, XBEE_RESET_BITPOS)
#define IO_clr_XBEE_RESET()		PORT_ClearOutputBit( &XBEE_RESET_PORT, XBEE_RESET_BITPOS)

//------------------------------------------------------------------------------------
// FLASH MEMORY SELECT

#define SPIMEM_CS_BITPOS		4
#define SPIMEM_CS_PORT			PORTC
#define IO_config_SPIMEM_CS()	PORT_SetPinAsOutput( &SPIMEM_CS_PORT, SPIMEM_CS_BITPOS)
#define IO_set_SPIMEM_CS()		PORT_SetOutputBit( &SPIMEM_CS_PORT, SPIMEM_CS_BITPOS)
#define IO_clr_SPIMEM_CS()		PORT_ClearOutputBit( &SPIMEM_CS_PORT, SPIMEM_CS_BITPOS)

#define SPIMEM_MOSI_BITPOS		5
#define SPIMEM_MOSI_PORT		PORTC
#define IO_config_SPIMEM_MOSI()	PORT_SetPinAsOutput( &SPIMEM_MOSI_PORT, SPIMEM_MOSI_BITPOS)
#define IO_set_SPIMEM_MOSI()	PORT_SetOutputBit( &SPIMEM_MOSI_PORT, SPIMEM_MOSI_BITPOS)
#define IO_clr_SPIMEM_MOSI()	PORT_ClearOutputBit( &SPIMEM_MOSI_PORT, SPIMEM_MOSI_BITPOS)

#define SPIMEM_SCK_BITPOS		7
#define SPIMEM_SCK_PORT			PORTC
#define IO_config_SPIMEM_SCK()	PORT_SetPinAsOutput( &SPIMEM_SCK_PORT, SPIMEM_SCK_BITPOS)
#define IO_set_SPIMEM_SCK()		PORT_SetOutputBit( &SPIMEM_SCK_PORT, SPIMEM_SCK_BITPOS)
#define IO_clr_SPIMEM_SCK()		PORT_ClearOutputBit( &SPIMEM_SCK_PORT, SPIMEM_SCK_BITPOS)

//void IO_config_SPIMEM_CS(void);
//void IO_set_SPIMEM_CS(void);
//void IO_clr_SPIMEM_CS(void);

//------------------------------------------------------------------------------------
// IO data pines

int8_t IO_read_DIN( uint8_t pin);
int8_t IO_set_DOUT(uint8_t pin);
int8_t IO_clr_DOUT(uint8_t pin);
int8_t IO_reflect_DOUTPUTS(uint8_t output_value );

//------------------------------------------------------------------------------------

#endif /* SRC_SPXR3_IO8_LIBS_L_IOPINES_H_ */
