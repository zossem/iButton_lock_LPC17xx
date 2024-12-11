#ifndef _SYS_TIC_TIMER_
#define _SYS_TIC_TIMER_
#include <stdint.h>

#define TICKS_IN_SECOND 1000000
#define TICKS_IN_MILISECOND 1000
#define TICKS_IN_MICROSECOND 1

void SysTick_Initialize(void); 
//void SysTick_Handler(void); 

/** Waits for a time equal to the input value (us) milliseconds.
  * While waiting, it goes to sleep mode.
  */
void delay_us(uint32_t us);

#endif
