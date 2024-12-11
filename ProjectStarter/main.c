#include "LPC17xx.h"
#include "PIN_LPC17xx.h"

#include <stdio.h>

#include "one_wire.h"
#include "uart0.h"
#include "delay.h"
#include "flash_operations.h"

void start(void);
void forever(void);

void testDelay(void);


int main(void)
{
	start();
	forever();
}

void start(void)
{
	SystemInit(); //function configures the oscillator (PLL) 
	UART0_Initialize();
	//prepare_sector(7);
}

void forever(void)
{
	char bfr[31];
	while (true)
	{
		testDelay(); // never ending function - to see if delay duration is 1 us
		send_UART_string("jamnik");
		// Reading the serial number
		uint8_t serial_number[8];
		int isOK = read_serial_number(serial_number);
		
		if(!isOK)
		{
			for(unsigned int i=0; i<8; i++)
			{
				sprintf(bfr, "%c ", serial_number[i]);
				send_UART_string(bfr);
			}
		}
		else
		{
			sprintf(bfr, "%d ", isOK);
			send_UART_string(bfr);
		}
	}
	
}

void testDelay(void)
{
	while (true)
	{
		LPC_GPIO0->FIODIR |= (1 << ONE_WIRE_PIN); // Setting pin as output
		LPC_GPIO0->FIOCLR = (1 << ONE_WIRE_PIN);  // Setting pin as low
		delay_us(1);
		LPC_GPIO0->FIODIR &= ~(1 << ONE_WIRE_PIN); // Setting pin as input
		delay_us(1);
	}
}
