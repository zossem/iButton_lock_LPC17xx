#ifndef _FLASH_OPERATIONS_
#define _FLASH_OPERATIONS_

//w przypadku korzystania z malloca heap musi zostac ustawiony
//(minimum 256 bajtow, bo tyle ma najmniejszy zapis)
//(Jako ze korzystamy z sektorow 4kB, potrzebujemy minimum tyle na heapie)
//w przypadku z korzystania ze zwyklych tablic nalezy zwiekszyc stack
//ale nie polecam bo latwo zapchac (malloc GUROM)

#include "LPC17xx.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define BUTTON_REGISTER 10
#define MAINTANANCE_REGISTER 11
#define HISTORY_REGISTER 12
#define R_NUM_OFFSET 8 //number of registered capsule offset in memory
#define H_NUM_OFFSET 16 //number of history entries offset in memory

void flash_test(void);
//Checks wheather flash already has data, if not, then prepares everything for the use
int initialize_flash();

/** Checks whether the iButton is in the database of iButtons that have access to the lock.
  * Returns: true - access, false - no access.
  */
bool is_registered(uint8_t serial_number[]);

/** Adding a iButton to the history with the time of addition.
  * Serial number has 8 elements, date has 6 elements (year, month, day, hour, minutes, seconds).
  * If there is no in history space, overwrite the oldest one.
  */
int add_history(uint8_t serial_number[], uint8_t date[]);

/** Adding a iButton to the database.
  * Serial number has 8 elements.
  * Returns: 0 - success, 1 - out of space.
  */
int add_iButton(uint8_t serial_number[]);

/** Prints the history of all accesses
  */
int print_history();

/** Delete a iButton from the database.
  * Serial number has 8 elements.
  * Returns: 0 - success, 1 - not found iButton.
  */
int delete_iButton(uint8_t serial_number[]);
#endif
