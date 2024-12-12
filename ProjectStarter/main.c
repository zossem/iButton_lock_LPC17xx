#include "LPC17xx.h"
#include "PIN_LPC17xx.h"

#include <stdio.h>

#include "one_wire.h"
#include "uart0.h"
//#include "delay.h"
#include "SysTick_timer.h"
#include "real_time_clock.h"
#include "flash_operations.h"

void start(void);
void forever(void);

void testDelay(int us);
void testRTC(void);
void read_time_from_UART(int *hour, int *min, int *sec);
int my_atoi(const char *str);



int main(void)
{
	start();
	forever();
}

void start(void)
{
	SystemInit(); //function configures the oscillator (PLL) 
	UART0_Initialize();
	SysTick_Initialize();
	/*
	int hour, min, sec;
	read_time_from_UART(&hour, &min, &sec);
	RTC_Initialize(hour, min, sec);
	*/
	//prepare_sector(7);
}

void forever(void)
{
	char bfr[31];
	while (true)
	{
		//send_UART_string("jamnik\n\r");
		//testDelay(10); // never ending function - to see if delay duration is 1 us
		//testRTC();
		
		// Reading the serial number
		uint8_t serial_number[8];
		int isOK = read_serial_number(serial_number);
		
		if(!isOK)
		{
			for(unsigned int i=0; i<8; i++)
			{
				sprintf(bfr, "%d ", serial_number[i]);
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

void testDelay(int us)
{
	send_UART_string("start deley test\n\r");
	while (true)
	{
		LPC_GPIO0->FIODIR |= (1 << ONE_WIRE_PIN); // Setting pin as output
		LPC_GPIO0->FIOCLR = (1 << ONE_WIRE_PIN);  // Setting pin as low

		delay_us(us);

		LPC_GPIO0->FIODIR &= ~(1 << ONE_WIRE_PIN); // Setting pin as input
		delay_us(us);
	}
}

void testRTC(void)
{
	char bfr[31];
	Time currentTime;
	// Odczyt aktualnego czasu z RTC
	currentTime = RTC_GetTime();

	// Wyswietlenie aktualnego czasu
	sprintf(bfr, "Time: %02d:%02d:%02d\n", currentTime.hour, currentTime.minute, currentTime.second);
	send_UART_string(bfr);
}

void read_time_from_UART(int *hour, int *min, int *sec)
{
    char buffer[3];

    // Wyslanie komunikatu do uzytkownika
    send_UART_string("Podaj czas w formacie HHMMSS:\r\n");

    // Odczyt godziny
    for (int i = 0; i < 2; i++) {
        buffer[i] = read_UART_char();
    }
    buffer[2] = '\0';
    *hour = my_atoi(buffer);

    // Odczyt minut
    for (int i = 0; i < 2; i++) {
        buffer[i] = read_UART_char();
    }
    buffer[2] = '\0';
    *min = my_atoi(buffer);

    // Odczyt sekund
    for (int i = 0; i < 2; i++) {
        buffer[i] = read_UART_char();
    }
    buffer[2] = '\0';
    *sec = my_atoi(buffer);
}

int my_atoi(const char *str)
{
    int result = 0;
    while (*str) {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

