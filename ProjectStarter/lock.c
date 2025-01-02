#include "lock.h"
#include <Board_LED.h> 
#include "SysTick_timer.h"

void lock_Initialize()
{
	LED_Initialize();
	for (int i = 0; i < (int)LED_GetCount(); i++)
	{
		LED_Off(i);
	} 
}

void open_lock()
{
	for(int j=0; j < 4; j++)
	{
		for (int i = 0; i < (int)LED_GetCount(); i++)
		{
			LED_On(i);
			delay_us(1250000);
		}
	
		for (int i = 0; i < (int)LED_GetCount(); i++)
		{
			LED_Off(i);
			delay_us(1250000);
		}
	}	
}