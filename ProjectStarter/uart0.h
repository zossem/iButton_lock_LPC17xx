#ifndef _UART0_
#define _UART0_

void send_UART_string(char * word);
char read_UART_char(void);
void UART0_Initialize(void);

#endif