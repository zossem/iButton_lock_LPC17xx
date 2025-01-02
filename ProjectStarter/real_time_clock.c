#include "LPC17xx.h"
#include "real_time_clock.h"


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
    currentTime.year = LPC_RTC->YEAR;
    currentTime.month = LPC_RTC->MONTH;
    currentTime.day = LPC_RTC->DOM;
    currentTime.hour = LPC_RTC->HOUR;
    currentTime.minute = LPC_RTC->MIN;
    currentTime.second = LPC_RTC->SEC;
    return currentTime;
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