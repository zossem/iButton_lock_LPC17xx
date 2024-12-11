#include "SysTick_timer.h"
#include "LPC17xx.h"

#include "uart0.h"
#include <stdio.h>

volatile uint32_t Ticks = 0; 

void SysTick_Initialize(void)
{
	SysTick_Config(SystemCoreClock / TICKS_IN_SECOND);
}

void SysTick_Handler(void)
{
	Ticks++;
}

void delay_us(uint32_t us)
{
	uint32_t ticks_on_start = Ticks;
	uint32_t ticks_on_end = Ticks + (us * TICKS_IN_MICROSECOND); 
	uint32_t current_ticks;
	
	if(ticks_on_end >= ticks_on_start) // Check if the end time has not exceeded the timer range
	{
		do
		{
			//__WFI(); // Sleep until next interrupt
			current_ticks = Ticks; // Protects against errors from changing the tick counter while checking the below conditions
		}while(!(current_ticks >= ticks_on_end || current_ticks < ticks_on_start)); // Exits loop if time has expired
	}
	else
	{
		do
		{
			//__WFI(); // Sleep until next interrupt
			current_ticks = Ticks; // Protects against errors from changing the tick counter while checking the below conditions
		}while(!(current_ticks >= ticks_on_end && current_ticks < ticks_on_start)); // Exits loop if time has expired
	}
}

