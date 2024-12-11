#include "LPC17xx.h"
#include "real_time_clock.h"


void RTC_Initialize(int hour, int min, int sec) 
{
    // Wlaczenie zasilania dla RTC
    LPC_SC->PCONP |= (1 << 9);

    // Wlaczenie zegara RTC
    LPC_RTC->CCR = (1 << 0) | (1 << 4);

    // Ustawienie poczatkowego czasu
    LPC_RTC->HOUR = hour;
    LPC_RTC->MIN = min;
    LPC_RTC->SEC = sec;

    // Wlaczenie aktualizacji czasu
    LPC_RTC->CCR |= (1 << 0);
}



Time RTC_GetTime(void)
{
    Time currentTime;
    currentTime.hour = LPC_RTC->HOUR;
    currentTime.minute = LPC_RTC->MIN;
    currentTime.second = LPC_RTC->SEC;
    return currentTime;
}