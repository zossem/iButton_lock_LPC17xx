#include "application_mode.h"
#include "LPC17xx.h"

void mode_Initialize(void)
{
	LPC_PINCON->PINMODE4 &= ~(3 << 26); // Clears bits 26 and 27 - responsible for pin P2.13
	LPC_PINCON->PINMODE4 |= (0<<26); // Sets bits 26 and 27 to 00 - pull-up
}

int get_mode(void)
{
	if(LPC_GPIO2->FIOPIN & (1 << MODE_PIN))
	{
		//high state - work mode
		return WORK_MODE;
	}
	else
	{
		//low state - configure mode
		return CONFIG_MODE;
		
	}
}