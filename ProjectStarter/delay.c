#include "delay.h"
#include "LPC17xx.h"

void delay_us(uint32_t us) {
    // Ustawienie preskalera na 1 mikrosekunde
    LPC_TIM0->PR = SystemCoreClock / 1000000 - 1; // (99 dla 100MHz)
    LPC_TIM0->MR0 = us; // Ustawienie wartosci w rejestrze MR0
    LPC_TIM0->MCR = 0x04; // Resetowanie TC po osiagnieciu wartosci MR0
    LPC_TIM0->TCR = 0x02; // Reset timera
    LPC_TIM0->TCR = 0x01; // Wlaczenie timera

    // Czekanie na uplyniecie czasu
    while (!(LPC_TIM0->IR & 0x01));

    LPC_TIM0->IR = 0x01; // Wyczyszczenie flagi przerwania
    LPC_TIM0->TCR = 0x00; // Wylaczenie timera
}    