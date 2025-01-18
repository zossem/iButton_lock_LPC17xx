#include "uart0.h"
#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include <string.h>


LPC_UART_TypeDef * p_UART0=LPC_UART0;


void UART2_Initialize(void)
{
	LPC_SC->PCONP |= (1 << 24); //wlaczenie uart2 (tylko uart 0 po resecie dziala)
	PIN_Configure(0, 10, PIN_FUNC_1, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL); //TXD2
	PIN_Configure(0, 11, PIN_FUNC_1, PIN_PINMODE_TRISTATE, PIN_PINMODE_NORMAL); //RXD2
	
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
	
	char echo[1];
	echo[0]=recived;
	send_UART_string(echo);	//Printing recived char
	
	return recived;
}
