#ifndef _ONE_WIRE_
#define _ONE_WIRE_

#include <stdint.h>
#include <stdbool.h>
// should be no interupts exept SysTick while reading and writing on 1-Wire bus
// pull up RESISTOR 5 kOhm needed

#define ONE_WIRE_PIN 15 // selected pin GPIO

#define POLYNOMIAL 0x8C // Polynomial: X^8 + X^5 + X^4 + 1

/** Calculate checksum - CRC.
  * Input: data - read serial number, length - number of bytes of family number and device unique number
  * Output: calculated CRC
  */
uint8_t calculate_crc(uint8_t data[], uint8_t length);


/** Checks the CRC. Calls the function that calculates checksum (calculate_crc) and compares the result with the read CRC.
  * Input: data - read serial number
  * Output: true - CRC is valid, false - CRC is invalid
  */
bool check_crc(uint8_t data[]);

/** Sends a bus reset signal.
  * Waits for the signal of the slave's presence.
  * Returns whether a slave is present: true - present, false - not present.
  */
bool one_wire_reset(void);


/** Sends a bit on the 1-Wire bus.
  */
void one_wire_write_bit(uint8_t bit);


/** Reads a bit from the 1-Wire bus.
  */
uint8_t one_wire_read_bit(void);


/** Sends a byte on the 1-Wire bus.
  */
void one_wire_write_byte(uint8_t byte);


/** Reads a byte from the 1-Wire bus.
  */
uint8_t one_wire_read_byte(void);



/** Reads a serial number (64 bits) of device.
  * Returns 0 if everything is OK.
  * Returns -1 if there is no slave present.
  * Returns -2 if checksum is incorrect.
  */
int read_serial_number(uint8_t serial_number[]);


#endif
