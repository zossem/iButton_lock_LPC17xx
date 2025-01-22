#include "LPC17xx.h"
#include "real_time_clock.h"
#include "uart0.h"

void RTC_Initialize(int year, int month, int day, int hour, int min, int sec) 
{
    // Wlaczenie zasilania dla RTC
    LPC_SC->PCONP |= (1 << 9);

    // Wlaczenie zegara RTC
    LPC_RTC->CCR = (1 << 0) | (1 << 4);

    // Ustawienie poczatkowego czasu
    LPC_RTC->YEAR = year;
    LPC_RTC->MONTH = month;
    LPC_RTC->DOM = day; //Day of month
    LPC_RTC->HOUR = hour;
    LPC_RTC->MIN = min;
    LPC_RTC->SEC = sec;

    // Wlaczenie aktualizacji czasu
    LPC_RTC->CCR |= (1 << 0);
}



Time RTC_GetTime(void)
{
    Time currentTime;
    currentTime.year = ( LPC_RTC->YEAR - 2000 );
    currentTime.month = LPC_RTC->MONTH;
    currentTime.day = LPC_RTC->DOM;
    currentTime.hour = LPC_RTC->HOUR;
    currentTime.minute = LPC_RTC->MIN;
    currentTime.second = LPC_RTC->SEC;
    return currentTime;
}


void read_time_from_UART(int *year, int *month, int *day, int *hour, int *min, int *sec)
{
		__disable_irq();
    char buffer[3];
		send_UART_string("Enter the day in YYMMDD format:\r\n");
	
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
    send_UART_string("Enter the time in HHMMSS format:\r\n");

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
/*
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
*/