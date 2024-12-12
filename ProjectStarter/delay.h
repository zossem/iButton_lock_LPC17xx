#ifndef _DELAY_
#define _DELAY_

#include <stdint.h>

#define SBIT_MR0I    0
#define SBIT_MR0R    1
#define SBIT_CNTEN   0
#define SBIT_CNTCLR  1

#define PCLK_TIMER0  2


void delay_us(uint32_t us);
void TIMER0_IRQHandler(void);
void TIMER0_Init(void);
unsigned int getPrescalar(uint8_t timerPclkBit);

#endif