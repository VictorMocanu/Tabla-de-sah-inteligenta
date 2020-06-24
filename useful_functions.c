/**
   * File name: useful_functions.c
   * File type: source code file (.c)
*/

#include "useful_functions.h"

/*----------------------------------------------------------------------------------------------------*/
/* 1) I/O ports - data types and functions: */
/* 1.3) */
void portPin_setType(volatile uint8_t *ddr_name, uint8_t ddr_pin_name, PortType port_type)
{
	if(port_type == INPUT)
	{
		*ddr_name &= ~(1 << ddr_pin_name);
	}
	else
	{
		if(port_type == OUTPUT)
		{
			*ddr_name |= (1 << ddr_pin_name);
		}
		else
		{
			
		}
	}
}

/* 1.4) */
void portPin_setValue(volatile uint8_t *port_name, uint8_t port_pin_name, PortValue port_value)
{
	if(port_value == LOW)
	{
		*port_name &= ~(1 << port_pin_name);
	}
	else
	{
		if(port_value == HIGH)
		{
			*port_name |= (1 << port_pin_name);
		}
		else
		{
			
		}
	}
}
/*----------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------*/
/* 2) MAX7219 & 7-segment display - functions (implement SPI protocol): */
/* 2.2) */
void SPI_masterInit(void)
{
	DDR_SPI_extension |= ((1 << DDR_SS_transmit) | (1 << DDR_SS_receive_control));    /* Set master SS_transmit and SS_receive_control as output */
	DDR_SPI |= ((1 << DDR_SS_receive) | (1 << DDR_SCK) | (1 << DDR_MOSI));            /* Set SCK and MOSI as output, SS_receive and MISO as input */
	PORT_SPI_extension |= ((1 << PIN_SS_transmit) | (1 << PIN_SS_receive_control));   /* Disable master SS_transmit and SS_receive_control pins (both active low) */
	SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);                                   /* Enable SPI, configure ATmega2560 as master device, set the clock rate at f_sys_ck/16 */
	SPCR |= (1 << CPOL);
	SPCR &= ~(1 << CPHA);
	/* Take data on the rising edge of ck(SPI data format), data order is MSB first */
}

/* 2.3) */
void MAX7219_SPI_masterTransmit(uint8_t address_SPI, uint8_t data_SPI)
{
	PORT_SPI_extension &= ~(1 << PIN_SS_transmit);					      /* Start SPI transmission, master pull SS low */
	SPDR = address_SPI;                                                   /* Transmit address byte */
	while(!(SPSR & (1 << SPIF)));                                         /* Wait for transmission complete */
	SPDR = data_SPI;                                                      /* Transmit data byte */
	while(!(SPSR & (1 << SPIF)));                                         /* Wait for transmission complete */
	PORT_SPI_extension |= (1 << PIN_SS_transmit);					      /* End SPI transmission, master pull SS high(other devices can be the master device) */
}

/* 2.4) */
void MAX7219_SPI_configureDisplay_decoded(void)
{
	SPI_masterInit();
	MAX7219_SPI_masterTransmit(0x0C, 0x01);				//Configure shutdown register to normal mode
	_delay_ms(10);
	MAX7219_SPI_masterTransmit(0x0A, 0x07);				//Configure intensity register to obtain ((15/32) * maximum intensity) of the display
	_delay_ms(10);
	MAX7219_SPI_masterTransmit(0x0B, 0x07);				//Configure scan-limit register to display all digits
	_delay_ms(10);
	MAX7219_SPI_masterTransmit(0x09, 0xFF);				//Configure decode register to enable decode mode
	_delay_ms(10);
	MAX7219_SPI_masterTransmit(0x0F, 0x00);				//Configure display test register to disable test mode
	_delay_ms(10);
}

/*----------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------*/
/* 3) SN74HC165N Parallel input - serial output (PISO) shift register - data types and functions: */
/* 3.2) */
void SN74HC165_init(void)
{
	DDR_SPI_extension |= (1 << DDR_SN74HC165_LD_control);  /* Set the pin connected to SN74HC165_LD as output. */
	PORT_SPI_extension |= (1 << PIN_SN74HC165_LD_control); /* Disable SN74HC165_LD input (active low). */
}

/* 3.3) */
void SN74HC165_loadBytes(void)
{
	PORT_SPI_extension &= ~(1 << PIN_SN74HC165_LD_control);  /* Generate a pulse on SN74HC165_LD input to load data in SN74HC165 shift register. */
	_delay_us(10);
	PORT_SPI_extension |= (1 << PIN_SN74HC165_LD_control);
}

/* 3.4) */
char SN74HC165_SPI_masterReceive(void)
{
	char SPI_data_received;
	PORT_SPI_extension &= ~(1 << PIN_SS_receive_control);                 /* Start SPI reception, master pull SS low */
	/* (emulating this command because the slave device does not have a dedicated SPI interface) */
	SPDR = 0x00;                                                          /* Send dummy data to take the received useful data */
	while(!(SPSR & (1 << SPIF)));                                         /* Wait for transmission complete */
	SPI_data_received = SPDR;                                             /* Temporary storage of the received data */
	PORT_SPI_extension |= (1 << PIN_SS_receive_control);                  /* End SPI reception, master pull SS high  */
	return SPI_data_received;                                             /* Return received data */
}

/* 3.5) */
uint64_t SN74HC165_SPI_masterReceive_eightBytes(void)
{
	/*---------------------------------------------------------------------------------------------------------------------------------------------------*/
	/* Chain 8 x SN74HC165, read 8 bytes through SPI and send them through SPI to MAX7219_7seg (each byte to each digit, from digit 7 to 0) */
	
	/* Initialize local variables */
	char receivedByte_SPI = 0x00;
	uint64_t receivedSensorData_SPI = 0x0000000000000000;
	
	/* Load the eight bytes read from the sensors into the 8 x SN74HC165 shift registers */
	SN74HC165_loadBytes();
	
	/* Read eight bytes from sensors through SPI and store them in the 64-bit variable receivedSensorData_SPI. */
	receivedByte_SPI = SN74HC165_SPI_masterReceive(); /* Read the FIRST BYTE */
	receivedSensorData_SPI = (((uint64_t)receivedByte_SPI) << (64 - 8)) | (receivedSensorData_SPI & 0x00FFFFFFFFFFFFFF); /* Apply 0x00FF_FFFF_FFFF_FFFF mask to CLEAR the FIRST BYTE from receivedSensorData_SPI, replace that byte with receivedByte_SPI */
	_delay_ms(1);
	
	receivedByte_SPI = SN74HC165_SPI_masterReceive(); /* Read the SECOND BYTE */
	receivedSensorData_SPI = (((uint64_t)receivedByte_SPI) << (64 - 16)) | (receivedSensorData_SPI & 0xFF00FFFFFFFFFFFF); /* Apply 0xFF00_FFFF_FFFF_FFFF mask to CLEAR the SECOND BYTE from receivedSensorData_SPI, replace that byte with receivedByte_SPI */
	_delay_ms(1);
	
	receivedByte_SPI = SN74HC165_SPI_masterReceive(); /* Read the THIRD BYTE */
	receivedSensorData_SPI = (((uint64_t)receivedByte_SPI) << (64 - 24)) | (receivedSensorData_SPI & 0xFFFF00FFFFFFFFFF); /* Apply 0xFFFF_00FF_FFFF_FFFF mask to CLEAR the THIRD BYTE from receivedSensorData_SPI, replace that byte with receivedByte_SPI */
	_delay_ms(1);
	
	receivedByte_SPI = SN74HC165_SPI_masterReceive(); /* Read the FOURTH BYTE */
	receivedSensorData_SPI = (((uint64_t)receivedByte_SPI) << (64 - 32)) | (receivedSensorData_SPI & 0xFFFFFF00FFFFFFFF); /* Apply 0xFFFF_FF00_FFFF_FFFF mask to CLEAR the FOURTH BYTE from receivedSensorData_SPI, replace that byte with receivedByte_SPI */
	_delay_ms(1);
	
	receivedByte_SPI = SN74HC165_SPI_masterReceive(); /* Read the FIFTH BYTE */
	receivedSensorData_SPI = (((uint64_t)receivedByte_SPI) << (64 - 40)) | (receivedSensorData_SPI & 0xFFFFFFFF00FFFFFF); /* Apply 0xFFFF_FFFF_00FF_FFFF mask to CLEAR the FIFTH BYTE from receivedSensorData_SPI, replace that byte with receivedByte_SPI */
	_delay_ms(1);
	
	receivedByte_SPI = SN74HC165_SPI_masterReceive(); /* Read the SIXTH BYTE */
	receivedSensorData_SPI = (((uint64_t)receivedByte_SPI) << (64 - 48)) | (receivedSensorData_SPI & 0xFFFFFFFFFF00FFFF); /* Apply 0xFFFF_FFFF_FF00_FFFF mask to CLEAR the SIXTH BYTE from receivedSensorData_SPI, replace that byte with receivedByte_SPI */
	_delay_ms(1);
	
	receivedByte_SPI = SN74HC165_SPI_masterReceive(); /* Read the SEVENTH BYTE */
	receivedSensorData_SPI = (((uint64_t)receivedByte_SPI) << (64 - 56)) | (receivedSensorData_SPI & 0xFFFFFFFFFFFF00FF); /* Apply 0xFFFF_FFFF_FFFF_00FF mask to CLEAR the SEVENTH BYTE from receivedSensorData_SPI, replace that byte with receivedByte_SPI */
	_delay_ms(1);
	
	receivedByte_SPI = SN74HC165_SPI_masterReceive(); /* Read the EIGHTH BYTE */
	receivedSensorData_SPI = (((uint64_t)receivedByte_SPI) << (64 - 64)) | (receivedSensorData_SPI & 0xFFFFFFFFFFFFFF00); /* Apply 0xFFFF_FFFF_FFFF_FF00 mask to CLEAR the EIGHTH BYTE from receivedSensorData_SPI, replace that byte with receivedByte_SPI */
	_delay_ms(1);
	
	/* Returns the 64-bit variable that represents the read values from the 64 sensors. */
	return receivedSensorData_SPI;
	/*---------------------------------------------------------------------------------------------------------------------------------------------------*/
}
/*----------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------*/
/* 4) WS2812 addresable LEDs and NRZ protocol - data types and functions: */
/* 4.2) */
void WS2812_MATRIX_setLedsBasedOnSensorValues(uint64_t sensorValues)
{
	/* Initialize local variables: */
	uint64_t mask = 0x8000000000000000; /* This mask is used to extract the current bit from each sensor read */
	uint64_t current_mask = 0x0000000000000000; /* The mask change based on the current sensor, store the current mask in current_mask variable */
	uint8_t sensor_current_bit = 0; /* Read the current bit from each sensor and store it in sensor_current_bit variable */
	
	/* Read each sensor value(16 sensors) and set the corresponding led ( Ex: Set led[2] if sensor2 = 0 ) */
	for(uint8_t i = 0; i < MAXPIX; i++)
	{
		current_mask = (mask >> i);
		sensor_current_bit = (uint8_t)((sensorValues & current_mask) >> (MAXPIX - i - 1));
		if(sensor_current_bit == 0)
		{
			led[i].r = 150;
			led[i].g = 50;
			led[i].b = 0;
		}
		else
		{
			led[i].r = 0;
			led[i].g = 0;
			led[i].b = 0;
		}
	}
	ws2812_setleds(led, MAXPIX); /* enable current led settings */
}

void WS2812_MATRIX_setLedsBasedOnSensorValuesAndColor(uint64_t sensorValues, uint8_t color)
{
	/* Initialize local variables: */
	uint64_t mask = 0x8000000000000000; /* This mask is used to extract the current bit from each sensor read */
	uint64_t current_mask = 0x0000000000000000; /* The mask change based on the current sensor, store the current mask in current_mask variable */
	uint8_t sensor_current_bit = 0; /* Read the current bit from each sensor and store it in sensor_current_bit variable */
	
	/* Read each sensor value(16 sensors) and set the corresponding led ( Ex: Set led[2] if sensor2 = 0 ) */
	for(uint8_t i = 0; i < MAXPIX; i++)
	{
		current_mask = (mask >> i);
		sensor_current_bit = (uint8_t)((sensorValues & current_mask) >> (MAXPIX - i - 1));
		if(sensor_current_bit == 0)
		{
			switch(color)
			{
				case RED:
				{
					led[i].r = 255;
					led[i].g = 0;
					led[i].b = 0;
					break;
				}
				case GREEN:
				{
					led[i].r = 0;
					led[i].g = 255;
					led[i].b = 0;
					break;
				}
				case BLUE:
				{
					led[i].r = 0;
					led[i].g = 0;
					led[i].b = 255;
					break;
				}
				case WHITE:
				{
					led[i].r = 255;
					led[i].g = 255;
					led[i].b = 255;
					break;
				}
				case YELLOW:
				{
					led[i].r = 255;
					led[i].g = 255;
					led[i].b = 0;
					break;
				}
				case PURPLE:
				{
					led[i].r = 255;
					led[i].g = 0;
					led[i].b = 255;
					break;
				}
				case TURQOISE:
				{
					led[i].r = 0;
					led[i].g = 255;
					led[i].b = 255;
					break;
				}
				case ORANGE:
				{
					led[i].r = 150;
					led[i].g = 50;
					led[i].b = 0;
					break;
				}
				case PINK_COLOR:
				{
					led[i].r = 255;
					led[i].g = 0;
					led[i].b = 128;
					break;
				}
				default:
				{
					//BLUE
					led[i].r = 0;
					led[i].g = 0;
					led[i].b = 255;
				}
			}
			
		}
		else
		{
			led[i].r = 0;
			led[i].g = 0;
			led[i].b = 0;
		}
	}
	ws2812_setleds(led, MAXPIX); /* enable current led settings */
}
/*----------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------*/
/* 5) Buttons - data types and functions: */
/* 5.2) */
void Buttons_init(void)
{
	DDR_Buttons &= ~((1 << DDR_Button7_pawn_promote_queen) | (1 << DDR_Button6_pawn_promote_rook) | (1 << DDR_Button5_pawn_promote_bishop) | (1 << DDR_Button4_pawn_promote_knight) |
	(1 << DDR_Button3_startGame) | (1 << DDR_Button2_confirm_black_move) | (1 << DDR_Button1_confirm_white_move)); /* Set PC6, PC5 and PC4 as inputs */
	PORT_Buttons |= (1 << PIN_Button7_pawn_promote_queen) | (1 << PIN_Button6_pawn_promote_rook) | (1 << PIN_Button5_pawn_promote_bishop) | (1 << PIN_Button4_pawn_promote_knight) |
	(1 << PIN_Button3_startGame) | (1 << PIN_Button2_confirm_black_move) | (1 << PIN_Button1_confirm_white_move); /* Enable PC6, PC5 and PC4 internal pull-up resistors */
}
/* 5.3) */
uint8_t Button_debounce(volatile uint8_t *portForRead_name, uint8_t port_pin_name)
{
	uint8_t button_currentState = 1;
	if(!((*portForRead_name) & (1 << port_pin_name)))
	{
		//_delay_ms(150);
        _delay_ms(80);
		if(!((*portForRead_name) & (1 << port_pin_name)))
		{
			button_currentState = 0;
		}
		/* //In caz ca se tine butonul apasat (aici nu e necesar fiindca acest lucru nu afecteaza logica ceasului de sah) 
		_delay_ms(150);
		if(!((*portForRead_name) & (1 << port_pin_name)))
		{
			button_currentState = 1;
		}
		*/
	}
	else
	{
		button_currentState = 1;
	}
	return button_currentState;
}
/*----------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------*/
/* 6) Timer1 and chess clock implementation - data types and functions: */
/* 6.2) */
void ATMEGA2560_TIMER1_init(void)
{
	TCCR1B |= (1 << WGM12); /* Configure timer1 in CTC mode */
	TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); /* Configure timer1 clock to no clock source in order to disable timer1 */
	TCNT1 = 0; /* Initialize timer1 register. */
	OCR1A = 19999; /* Configure timer1 output compare match A register with the converted value */
	TIMSK1 |= (1 << OCIE1A); /* Enable output compare match A interrupt */
}
/* 6.3) */
void ATMEGA2560_TIMER1_start(void)
{
	TCCR1B |= (1 << CS11); /* Configure timer1 with internal clock and prescaler = 8 in order to enable timer1 */
}
/* 6.4) */
void ATMEGA2560_TIMER1_stop(void)
{
	TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); /* Configure timer1 clock to no clock source in order to disable timer1 */
}
/* 6.5) */
void MAX7219_SPI_displayWhiteTime(uint8_t main_global_white_minutes, uint8_t main_global_white_seconds)
{
	MAX7219_SPI_masterTransmit(0x05, (main_global_white_seconds % 10));
	MAX7219_SPI_masterTransmit(0x06, ((main_global_white_seconds / 10) % 10));
	MAX7219_SPI_masterTransmit(0x07, (0x80 | (main_global_white_minutes % 10)));
	MAX7219_SPI_masterTransmit(0x08, ((main_global_white_minutes / 10) % 10));
}
/* 6.6) */
void MAX7219_SPI_displayBlackTime(uint8_t main_global_black_minutes, uint8_t main_global_black_seconds)
{
	MAX7219_SPI_masterTransmit(0x01, (main_global_black_seconds % 10));
	MAX7219_SPI_masterTransmit(0x02, ((main_global_black_seconds / 10) % 10));
	MAX7219_SPI_masterTransmit(0x03, (0x80 | (main_global_black_minutes % 10)));
	MAX7219_SPI_masterTransmit(0x04, ((main_global_black_minutes / 10) % 10));
}
/*----------------------------------------------------------------------------------------------------*/