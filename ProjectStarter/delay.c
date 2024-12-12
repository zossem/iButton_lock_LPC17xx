#include "delay.h"
#include "LPC17xx.h"

/* https://www.ocfreaks.com/lpc1768-timer-programming-tutorial/ */

volatile bool LPC_TIM0_waitfor;

void delay_us(uint32_t us) 
{
    LPC_TIM0_waitfor = false;

    LPC_TIM0->MR0  = us;    /* Load timer value to generate us delay*/    
    LPC_TIM0->MCR  = (1<<SBIT_MR0I) | (1<<SBIT_MR0R);     /* Clear TC on MR0 match and Generate Interrupt*/

    NVIC_EnableIRQ(TIMER0_IRQn);     /* Enable Timer0 Interrupt */
    
    LPC_TIM0->TCR  = (1<<SBIT_CNTCLR); /* Clear timer */
    LPC_TIM0->TCR  = (1<<SBIT_CNTEN);  /* Start timer by setting the Counter Enable*/
    
    // Czekanie na uplyniecie czasu
    while (!(LPC_TIM0_waitfor));
}    


void TIMER0_IRQHandler(void)
{
    unsigned int isrMask;

    isrMask = LPC_TIM0->IR; 
    LPC_TIM0->IR = isrMask;         /* Clear the Interrupt Bit */
    LPC_TIM0->TCR = 0x00; /* Wylaczenie timera */
    
    LPC_TIM0_waitfor = true;
}

void TIMER0_Init(void)
{
    LPC_SC->PCONP |= (1<<SBIT_TIMER0); /* Power ON Timer0  By default TIM0 and TIM1 are enabled. */
    
    LPC_SC->PCLKSEL0 &= ~(0x3<<PCLK_TIMER0);   /* Timer0 bits[3:2] in PCLKSEL0 = [00] – PCLK = CCLK/4 (Default after reset) */
    LPC_SC->PCLKSEL0 |= (0x1<<PCLK_TIMER0);    /* Timer0 bits[3:2] in PCLKSEL0 = [01] – PCLK = CCLK */

    LPC_TIM0->CTCR = 0x0;  /*  Timer Mode */
    LPC_TIM0->PR =  getPrescalar(PCLK_TIMER0)/1000000 - 1;     /* Prescalar for 1us (1000000Counts/sec) */
}


unsigned int getPrescalar(uint8_t timerPclkBit)
{
    unsigned int pclk,prescalarForUs;
    pclk = (LPC_SC->PCLKSEL0 >> timerPclkBit) & 0x03;  /* get the pclk info for required timer */

    switch ( pclk )                                    /* Decode the bits to determine the pclk*/
    {
    case 0x00:
        pclk = SystemCoreClock/4;
        break;

    case 0x01:
        pclk = SystemCoreClock; 
        break; 

    case 0x02:
        pclk = SystemCoreClock/2;
        break; 

    case 0x03:
        pclk = SystemCoreClock/8;
        break;

    default:
        pclk = SystemCoreClock/4;
        break;  
    }

    return pclk;
}