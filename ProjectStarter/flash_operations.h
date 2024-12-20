#ifndef _FLASH_OPERATIONS_
#define _FLASH_OPERATIONS_

//w przypadku korzystania z malloca heap musi zostac ustawiony
//(minimum 256 bytow, bo tyle ma najmniejszy zapis)
//w przypadku z korzystania ze zwyklych tablic nalezy zwiekszyc stack
//ale nie polecam bo latwo zapchac (malloc GUROM)

#include "LPC17xx.h"
#include <stdbool.h>
#include <stdint.h>

uint32_t get_flash_sector_address(uint32_t sector_number);
bool prepare_sector(uint32_t sector_number);
bool erase_sector(uint32_t sector_number);
bool is_sector_empty(uint32_t sector_number);
bool write_to_flash_sector(uint32_t sector_number, uint8_t *data, uint32_t size);
bool read_from_flash(uint32_t sector_number, uint8_t *buffer, uint32_t size);
bool verify_flash_data(uint8_t *data, uint8_t *buffer, uint32_t size);

#endif
