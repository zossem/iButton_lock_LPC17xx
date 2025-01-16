#include "LPC17xx.h"
#include "PIN_LPC17xx.h"

#include <stdio.h>
#include <stdlib.h>


#include "one_wire.h"
#include "uart0.h"
#include "SysTick_timer.h"
#include "real_time_clock.h"
#include "flash_operations.h"
#include "application_mode.h"
#include "lock.h"

void start(void);
void forever(void);

void read_time_from_UART(int *year, int *month, int *day, int *hour, int *min, int *sec);
int my_atoi(const char *str);

void setup_eint0(void); // Inicialize interrupts from key0
void setup_eint1(void); // Inicialize interrupts from key1

void flash_test(void);


int main(void)
{
	start();
	uint8_t a[8] = {0,1,2,3,4,5,6,7} ;
	if(add_iButton(a) == 0) send_UART_string("Zapisalo\n");
	if(is_registered(a) == true) send_UART_string("Jest w srodku\n");
	int out = delete_iButton(a);
	if(out == 0) send_UART_string("Usuniety\n");
	if(out == -1) send_UART_string("Bardzo zle\n");
	if(out == 1) send_UART_string("Zle\n");
	if(is_registered(a) == false) send_UART_string("Jest w srodku");
	while(1);
	//flash_test();
	//forever();
}

void start(void)
{
	SystemInit(); //function configures the oscillator (PLL) 
	UART0_Initialize();
	SysTick_Initialize();// Flash Timeout po odblokowaniu
	//(naprawione) przerwania przeszkadzaja flashowi, wiec w funkcjach na
	//flashu trzeba je wylaczyc (wylaczylem tez na uarcie)
	mode_Initialize();
	//lock_Initialize();// Nachodzi na piny uart0 (do poprawy)
	setup_eint0();
	setup_eint1();
	int year, month, day, hour, min, sec;
	//read_time_from_UART(&year, &month, &day, &hour, &min, &sec);
	//RTC_Initialize(year, month, day, hour, min, sec);
	
}

void forever(void)
{
	char bfr[31];
	
	
	while (true)
	{
		// Reading the serial number
		uint8_t serial_number[8];
		int is_SN_read_unsuccessfully = read_serial_number(serial_number);
			
		if(!is_SN_read_unsuccessfully)
		{
			send_UART_string("Read number: ");
			for(unsigned int i=0; i<8; i++)
			{
				sprintf(bfr, "%d ", serial_number[i]);
				send_UART_string(bfr);
			}
			send_UART_string(" CRC is correct\n\r");
				
			if(is_registered(serial_number)) // there is such serial number in database
			{
				// chceck mode of application
				if(get_mode() == WORK_MODE) // WORK MODE - opening lock or denying access
				{
					Time currentTime;
					// Read current time
					currentTime = RTC_GetTime();
					uint8_t date[6];
					date[0]=currentTime.year;
					date[1]=currentTime.month;
					date[2]=currentTime.day;
					date[3]=currentTime.hour;
					date[4]=currentTime.minute;
					date[5]=currentTime.second;
						
					open_lock();
					add_history(serial_number, date);
				}
				else	//CONFIG MODE - adding new iButtons
				{
					send_UART_string("This iButton is already registered\n\r");
				}
			}
			else // there is no such serial number in database
			{
				// chceck mode of application
				if(get_mode() == WORK_MODE) // WORK MODE - opening lock or denying access
				{
					send_UART_string("This iButton is not allowed to open this lock.\n\r");
				}
				else	//CONFIG MODE - adding new iButtons
				{
					int is_iButton_added_unsuccessfully = add_iButton(serial_number);
					if(!is_iButton_added_unsuccessfully)
					{
						send_UART_string("This iButton now has lock access.\n\r");
					}
					else if(is_iButton_added_unsuccessfully == 1)
					{
						send_UART_string("Error: The limit of iButtons that can access the lock has been reached. This iButton has not been granted access.\n\r");
					}
				}
			}	
		}
		else if(is_SN_read_unsuccessfully == -1)
		{
				send_UART_string("Slave is not present\n\r");
		}
		else if(is_SN_read_unsuccessfully == -2)
		{
				send_UART_string("CRC is incorrect\n\r");
		}
		
		delay_us(5000000);
		
	}	
}

void read_time_from_UART(int *year, int *month, int *day, int *hour, int *min, int *sec)
{
		__disable_irq();
    char buffer[3];
		send_UART_string("Podaj dzien w formacie YYMMDD:\r\n");
	
		// Odczyt roku
    for (int i = 0; i < 2; i++) {
        buffer[i] = read_UART_char();
    }
    buffer[2] = '\0';
    *year = 2000 + my_atoi(buffer);

    // Odczyt miesiaca
    for (int i = 0; i < 2; i++) {
        buffer[i] = read_UART_char();
    }
    buffer[2] = '\0';
    *month = my_atoi(buffer);

    // Odczyt sekund
    for (int i = 0; i < 2; i++) {
        buffer[i] = read_UART_char();
    }
    buffer[2] = '\0';
    *day = my_atoi(buffer);

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
		__enable_irq();
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

void setup_eint0(void) 
{
	LPC_PINCON->PINSEL4 |= (1 << 20); // Set up P2.10 as EINT0
	// Set: EINT0 is edge sensitive
	LPC_SC->EXTMODE |= (1 << 0);
	// Set: EINT0 is high-active or rising-edge sensitive
	LPC_SC->EXTPOLAR &= ~(1 << 0); 
	// Clear the EINT0 interrupt flag
	LPC_SC->EXTINT = (1 << 0);        

	// Enable the EINT0 interrupt in NVIC
	NVIC_EnableIRQ(EINT0_IRQn);      
}

void setup_eint1(void) 
{
	LPC_PINCON->PINSEL4 |= (1 << 22); // Set up P2.11 as EINT1
	// Set: EINT1 is edge sensitive
	LPC_SC->EXTMODE |= (1 << 1);
	// Set: EINT1 is high-active or rising-edge sensitive
	LPC_SC->EXTPOLAR &= ~(1 << 1); 
	// Clear the EINT1 interrupt flag
	LPC_SC->EXTINT = (1 << 1);        

	// Enable the EINT1 interrupt in NVIC
	NVIC_EnableIRQ(EINT1_IRQn);      
}

void EINT0_IRQHandler(void)
{
	if (LPC_SC->EXTINT & (1 << 0)) // Check if the interrupt comes from EINT0
	{
		LPC_SC->EXTINT = (1 << 0);   // Clear the EINT0 interrupt flag
		
		print_history();
		
		send_UART_string("Click 1\r\n"); 
	}
} 

void EINT1_IRQHandler(void)
{
	if (LPC_SC->EXTINT & (1 << 1)) // Check if the interrupt comes from EINT1
	{
		LPC_SC->EXTINT = (1 << 1);   // Clear the EINT1 interrupt flag
		
		send_UART_string("Click 2\r\n");
		
		if(get_mode() == CONFIG_MODE)
		{
			send_UART_string("Insert the iButton you want to remove from the database.\r\n");
			
			bool is_present=false;
			uint8_t serial_number[8];
			
			for(int t=0; t < 60; t++) // Tries to read the iButton for approximately 60 seconds
			{
				// Reading the serial number
				int is_SN_read_unsuccessfully = read_serial_number(serial_number);
					
				if(!is_SN_read_unsuccessfully)
				{
					send_UART_string("Read number: ");
					char bfr[31];
					for(unsigned int i=0; i<8; i++)
					{
						sprintf(bfr, "%d ", serial_number[i]);
						send_UART_string(bfr);
					}
					send_UART_string(" CRC is correct\n\r");
					is_present = true;
					break;
				}
				else if(is_SN_read_unsuccessfully == -1)
				{
					send_UART_string("Slave is not present\n\r");
				}
				else if(is_SN_read_unsuccessfully == -2)
				{
					send_UART_string("CRC is incorrect\n\r");
				}
				
				delay_us(10000000); //Wait 1 s
			}

			if(is_present)
			{
				int is_iButton_deleted_unsuccessfully = delete_iButton(serial_number);
				if(!is_iButton_deleted_unsuccessfully)
				{
					send_UART_string("This iButton has now lost lock access.\n\r");
				}
				else if(is_iButton_deleted_unsuccessfully == 1)
				{
					send_UART_string("Error: This iButton has not access before.\n\r");
				}
			}
			else // unpresent iButton
			{
				send_UART_string("Error: Failed to detect iButton.\n\r");
			}
		}		
	}
} 


// Test dziala poprawnie, jesli on dziala to znaczy ze jest dobrze
void flash_test(void)
{
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
	read_from_flash(sector_number, read_buffer, 256, 0);
	send_UART_string((char*)read_buffer);
	
	free(data_to_write);
	free(read_buffer);
}


