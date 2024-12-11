#include "uart0.h"
#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include <string.h>


LPC_UART_TypeDef * p_UART0=LPC_UART0;


void UART0_Initialize(void)
{
	PIN_Configure(0, 2, PIN_FUNC_1, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL); //TXD0
	PIN_Configure(0, 3, PIN_FUNC_1, PIN_PINMODE_TRISTATE, PIN_PINMODE_NORMAL); //RXD0
	
	p_UART0->LCR=0x83; // 8 bits, 1 stop, no parity DLAB=1
	p_UART0->DLM=0;
	p_UART0->DLL=163; //25MHz oscylator, 9600 baudrate
	p_UART0->LCR=0x03; // 8 bits, 1 stop, no parity DLAB=0
}


void send_UART_string(char * word)
{
	for(unsigned int i=0; i<strlen(word); i++)
	{
		while(!(p_UART0->LSR & 0x20));  // check transmiter hold register empty
		p_UART0->THR=word[i];
	}
}

char read_UART_char(void)
{
	while(!(p_UART0->LSR & 0x01));  // check if The UART1 receiver FIFO is not empty.
	char recived = p_UART0->RBR;
	return recived;
}
