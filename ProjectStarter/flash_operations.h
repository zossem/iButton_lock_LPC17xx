#ifndef _FLASH_OPERATIONS_
#define _FLASH_OPERATIONS_

#include "LPC17xx.h"
#include <stdbool.h>
#include <stdint.h>

bool prepare_sector(uint32_t sector_number);

#endif
