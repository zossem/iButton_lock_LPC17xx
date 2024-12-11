#include "LPC17xx.h"
#include "PIN_LPC17xx.h"

#include <stdio.h>

#include "one_wire.h"
#include "uart0.h"
#include "SysTick_timer.h"
#include "flash_operations.h"

void start(void);
void forever(void);


int main(void)
{
	start();
	forever();
}

void start(void)
{
	UART0_Initialize();
	SysTick_Initialize();
	//prepare_sector(7);
}

void forever(void)
{
	//uint32_t freq;
	char bfr[31];
	//char my_char ;
	while (true)
	{

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
		
		//volatile char received = p_UART0->RBR; //z  tam bedzie do oczytania
		//freq = SystemCoreClock;
		//sprintf(bfr, "%d", freq);
		//send_UART_string(bfr);
		
		/*my_char = read_UART_char();
		my_char+=1;
		sprintf(bfr, "%c%c%c", my_char, my_char, my_char);
		send_UART_string(bfr);*/
	}
	
}
