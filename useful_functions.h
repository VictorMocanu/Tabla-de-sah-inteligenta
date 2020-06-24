/**
   * File name: useful_functions.h
   * File type: header file (.h)
*/

#ifndef USEFUL_FUNCTIONS_H
#define USEFUL_FUNCTIONS_H

#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include "light_ws2812.c"

/*----------------------------------------------------------------------------------------------------*/
/* 1) I/O ports - data types and functions: */
/* 1.1) */
enum PortType {INPUT = 0, OUTPUT = 1};
typedef enum PortType PortType;
/* 1.2) */
enum PortValue {LOW = 0, HIGH = 1};
typedef enum PortValue PortValue;
/* 1.3) */
void portPin_setType(volatile uint8_t *ddr_name, uint8_t ddr_pin_name, PortType port_type); /* *ddr_name = &DDRn (Ex: &DDRB); ddr_pin_name = DDnx (Ex: DDB1); type = INPUT or OUTPUT; */
/* 1.4) */
void portPin_setValue(volatile uint8_t *port_name, uint8_t port_pin_name, PortValue port_value);  /* *port_name = &PORTn (Ex: &PORTB); port_pin_name = PINnx (Ex: PINB1); type = INPUT or OUTPUT; */
/*----------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------*/
/* 2) MAX7219 & 7-segment display - data types and functions (implement SPI protocol): */
/* 2.1) */
#define DDR_SPI DDRB
#define PORT_SPI PORTB
#define DDR_SS_receive DDB0
#define PIN_SS_receive PINB0
#define DDR_SPI_extension DDRA
#define PORT_SPI_extension PORTA
#define DDR_SS_transmit DDA0
#define PIN_SS_transmit PINA0
#define DDR_SS_receive_control DDA1
#define PIN_SS_receive_control PINA1
#define DDR_SCK DDB1
#define PIN_SCK PINB1
#define DDR_MOSI DDB2
#define PIN_MOSI PINB2
#define DDR_MISO DDB3
#define PIN_MISO PINB3
/* 2.2) */
void SPI_masterInit(void); /* Initialize SPI hardware block used for communication between: (ATMEGA2560 and MAX7219), (ATMEGA2560 and SN74HC165N). */
/* 2.3) */
void MAX7219_SPI_masterTransmit(uint8_t address_SPI, uint8_t data_SPI); /* Send two bytes through SPI (one for address and one for data) to allow entering commands for configuring and displaying informations on 7-segment display. */
/* 2.4) */
void MAX7219_SPI_configureDisplay_decoded(void); /* Configure MAX7219 registers to set the operating mode (normal/shutdown), the intensity value of the 7-segment display LEDs, the number of digits displayed, decoding mode (decoded/undecoded), test mode (enabled/disabled). */
/*----------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------*/
/* 3) SN74HC165N Parallel input - serial output (PISO) shift register - data types and functions: */
/* 3.1) */
#define DDR_SN74HC165_LD_control DDA7
#define PIN_SN74HC165_LD_control PINA7
/* 3.2) */
void SN74HC165_init(void); /* Initialize ATMEGA2560 pins connected to SN74HC165, pins that are not used for SPI (~LD - parallel load input pin). */
/* 3.3) */
void SN74HC165_loadBytes(void); /* Load input bytes (from 8 x SN74HC165 parallel inputs) in 8 x SN74HC165 registers. */
/* 3.4) */
char SN74HC165_SPI_masterReceive(void); /* Send one dummy byte in order to receive one byte through SPI. Read the received byte. */
/* 3.5) */
uint64_t SN74HC165_SPI_masterReceive_eightBytes(void); /* Receive 8 bytes through SPI. Read these bytes. */
/*----------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------*/
/* 4) WS2812 addresable LEDs and NZR protocol - data types and functions: */
/* 4.1) */
#define MAXPIX 64 // MODIFICAT ,INAINTE ERA 30 default, SE SETEAZA NR de LED-uri conectate in cascada
#define COLORLENGTH (MAXPIX/2)
#define FADE (256/COLORLENGTH)

struct cRGB led[MAXPIX];
/* WS2812 configuration ("light_ws2812.h"): */
/* #define ws2812_port A */
/* #define ws2812_pin 2 */

#define RED 0
#define GREEN 1
#define BLUE 2
#define WHITE 3
#define YELLOW 4
#define PURPLE 5
#define TURQOISE 6
#define ORANGE 7
#define PINK_COLOR 8

/* 4.2) */
void WS2812_MATRIX_setLedsBasedOnSensorValues(uint64_t sensorValues); /* Set 64 x WS2812 LEDs based on the 64-bits read from 64 x hall sensors. */
/*----------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------*/
/* 5) Buttons - data types and functions: */
/* 5.1) */
#define DDR_Buttons DDRC
#define PORT_Buttons PORTC
#define READ_PIN_Buttons PINC
#define DDR_Button1_confirm_white_move DDC5
#define PIN_Button1_confirm_white_move PINC5
#define DDR_Button2_confirm_black_move DDC4
#define PIN_Button2_confirm_black_move PINC4
#define DDR_Button3_startGame DDC6
#define PIN_Button3_startGame PINC6
#define DDR_Button4_pawn_promote_knight DDC3
#define PIN_Button4_pawn_promote_knight PINC3
#define DDR_Button5_pawn_promote_bishop DDC2
#define PIN_Button5_pawn_promote_bishop PINC2
#define DDR_Button6_pawn_promote_rook DDC1
#define PIN_Button6_pawn_promote_rook PINC1
#define DDR_Button7_pawn_promote_queen DDC0
#define PIN_Button7_pawn_promote_queen PINC0
/* 5.2) */
void Buttons_init(void); /* Initialization of the pins used to read the buttons: (Button1 -> PC4), (Button2 -> PC5), (Button3 -> PC6). */
/* 5.3) */
uint8_t Button_debounce(volatile uint8_t *portForRead_name, uint8_t port_pin_name); /* Read the current state of the button by selecting the corresponding button pin (solving the button bounce problem is included). */
/*----------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------*/
/* 6) Timer1 and chess clock implementation - data types and functions: */
/* 6.1) */
#define WHITE_MOVES 1
#define BLACK_MOVES 0
#define INITIAL_WHITE_MINUTES 0
#define INITIAL_WHITE_SECONDS 0
#define INITIAL_BLACK_MINUTES 0
#define INITIAL_BLACK_SECONDS 0
/* 6.2) */
void ATMEGA2560_TIMER1_init(void); /* Initialization of the timer1 internal block of ATMEGA2560. Configure timer1 mode, timer1 clock (no clock source = timer1 disabled), the value from which the timer1 starts counting, the value at which timer is reset, triggering the compare match ISR. Enable the compare match ISR. */
/* 6.3) */
void ATMEGA2560_TIMER1_start(void); /* Configure timer1 with internal clock and prescaler = 8 in order to enable timer1. */
/* 6.4) */
void ATMEGA2560_TIMER1_stop(void); /* Configure timer1 clock to no clock source in order to disable timer1 */
/* 6.5) */
void MAX7219_SPI_displayWhiteTime(uint8_t main_global_white_minutes, uint8_t main_global_white_seconds); /* Display the current white time (minutes and seconds) on the 7-segment display. */
/* 6.6) */
void MAX7219_SPI_displayBlackTime(uint8_t main_global_black_minutes, uint8_t main_global_black_seconds); /* Display the current black time (minutes and seconds) on the 7-segment display. */
/*----------------------------------------------------------------------------------------------------*/

#endif