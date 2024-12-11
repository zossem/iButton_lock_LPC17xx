#ifndef _REAL_TIME_CLOCK_
#define _REAL_TIME_CLOCK_

#include <stdint.h>

// Struktura do przechowywania czasu
typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} Time;

// Funkcja inicjalizujaca RTC
void RTC_Initialize(int hour, int min, int sec);

// Funkcja odczytujaca aktualny czas z RTC
Time RTC_GetTime(void);

#endif