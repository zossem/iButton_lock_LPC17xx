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


/** Checks whether the iButton is in the database of iButtons that have access to the lock.
  * Returns: true - access, false - no access.
  */
bool is_registered(uint8_t serial_number[]);

/** Adding a iButton to the history with the time of addition.
  * Serial number has 8 elements, date has 6 elements (year, month, day, hour, minutes, seconds).
  * If there is no in history space, overwrite the oldest one.
  */
void add_history(uint8_t serial_number[], uint8_t date[]);

/** Adding a iButton to the database.
  * Serial number has 8 elements.
  * Returns: 0 - success, 1 - out of space.
  */
int add_iButton(uint8_t serial_number[]);

/** Prints the history of all accesses
  */
void print_history();

/** Delete a iButton from the database.
  * Serial number has 8 elements.
  * Returns: 0 - success, 1 - not found iButton.
  */
int delete_iButton(uint8_t serial_number[]);
#endif
