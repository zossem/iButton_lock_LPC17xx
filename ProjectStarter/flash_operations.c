#include "flash_operations.h"
#include "uart0.h"
#include <stdio.h>

#define IAP_LOCATION 0x1FFF1FF1
typedef void (*IAP)(unsigned int[], unsigned int[]);
IAP iap_entry = (IAP) IAP_LOCATION;

#define SYSTEM_CLOCK_KHZ (SystemCoreClock / 1000)

bool prepare_sector(uint32_t sector_number)
{
		char bfr[40];
    unsigned int command[5];
    unsigned int result[5];
    command[0] = 50;  // Prepare Sector
    command[1] = sector_number;  // Numer sektora
    command[2] = sector_number;  // Numer sektora (tylko jeden sektor)
		send_UART_string("HI");
    iap_entry(command, result);
	
		sprintf(bfr, "%d", result[0]);
		send_UART_string(bfr);
    if (result[0] != 0) {
        return false;  // Obsluga bledu
    }
    return true; // Sukces
}