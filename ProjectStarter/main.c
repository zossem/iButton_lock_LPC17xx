#include "LPC17xx.h"
#include "PIN_LPC17xx.h"

#include <stdio.h>

#include "one_wire.h"
#include "uart0.h"
//#include "delay.h"
#include "SysTick_timer.h"
#include "real_time_clock.h"
#include "flash_operations.h"
#include "application_mode.h"

void start(void);
void forever(void);

void testDelay(int us);
void testRTC(void);
void read_time_from_UART(int *hour, int *min, int *sec);
int my_atoi(const char *str);
void flash_test(void);


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
	mode_Initialize();
	
	int hour, min, sec;
	read_time_from_UART(&hour, &min, &sec);
	RTC_Initialize(hour, min, sec);
	
	//prepare_sector(7);
}

void forever(void)
{
	char bfr[31];
	
	
	while (true)
	{
	/*
		//testRTC();
		if(get_mode() == WORK_MODE)
		{
			send_UART_string("Work mode\r\n");
		}
		else
		{
			send_UART_string("Config mode\r\n");
		}
		*/
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
		delay_us(5000000);
		
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
	sprintf(bfr, "Time: %02d:%02d:%02d\r\n", currentTime.hour, currentTime.minute, currentTime.second);
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

void flash_test(void)
{
	/*
    uint32_t sector_number = 16;
	
	uint8_t *data_to_write = (uint8_t*)malloc(sizeof(uint8_t) * 256);
	char* zosia = "zosia";
	for(int i = 0; zosia[i] != '\0'; i++)
	{
		data_to_write[i] = zosia[i];
		data_to_write[i+1] = '\0';
	}
	write_to_flash_sector(sector_number, data_to_write, 256);
	
	uint8_t *read_buffer = (uint8_t*)malloc(sizeof(uint8_t) * 256);
	read_from_flash(sector_number, read_buffer, 256);
	send_UART_string((char*)read_buffer);
	
	verify_flash_data(data_to_write, read_buffer, 256);
	
	free(data_to_write);
	free(read_buffer);
	*/
}
