#include "LPC17xx.h"
//#include "delay.h"
#include "SysTick_timer.h"
#include "one_wire.h"

#include "uart0.h"

uint8_t calculate_crc(uint8_t data[], uint8_t length) {
    uint8_t crc = 0; // Initialize the shift register to zero

    for (uint8_t i = 0; i < length; i++) {
        uint8_t byte = data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            bool mix = (crc ^ byte) & 0x01;
            crc >>= 1;
            if (mix) {
                crc ^= POLYNOMIAL;
            }
            byte >>= 1;
        }
    }

    return crc;
}

bool check_crc(uint8_t data[]) 
{
    // The data array should contain the family code, serial number, and CRC
    uint8_t crc = calculate_crc(data, 7); // Calculate CRC for the first 7 bytes (family code + serial number)
    return crc == data[7]; // Compare calculated CRC with the provided CRC
}

/** Sends a bus reset signal.
  * Waits for the signal of the slave's presence.
  * Returns whether a slave is present: true - present, false - not present.
  */
bool one_wire_reset(void) 
{
		//send_UART_string("one_wire_reset 0\n\r");
    // 1-Wire bus reset
    LPC_GPIO0->FIODIR |= (1 << ONE_WIRE_PIN); // Setting pin as output
    LPC_GPIO0->FIOCLR = (1 << ONE_WIRE_PIN);  // Setting pin as low
		//send_UART_string("one_wire_reset 0.5\n\r");
    delay_us(480);                            // Delay 480 microseconds - reset signal
	
		//send_UART_string("one_wire_reset 1\n\r");
    LPC_GPIO0->FIODIR &= ~(1 << ONE_WIRE_PIN); // Setting pin as input
    delay_us(70);                             // Delay 70 microseconds
	
		//send_UART_string("one_wire_reset 2\n\r");
    uint8_t presence = (LPC_GPIO0->FIOPIN & (1 << ONE_WIRE_PIN)) == 0;  // Check the device's response
	
    delay_us(410);                            // Delay 410 microseconds
		//send_UART_string("one_wire_reset 3\n\r");
	
	return presence;
}

/** Sends a bit on the 1-Wire bus.
  */
void one_wire_write_bit(uint8_t bit) 
{
    LPC_GPIO0->FIODIR |= (1 << ONE_WIRE_PIN); // Setting pin as output
    LPC_GPIO0->FIOCLR = (1 << ONE_WIRE_PIN);  // Setting pin as low
    delay_us(bit ? 10 : 65);                  // Delay depending on the bit value
	
    LPC_GPIO0->FIODIR &= ~(1 << ONE_WIRE_PIN); // Setting pin as input
    delay_us(bit ? 73 : 18);                   // Delay depending on the bit value
}

/** Reads a bit from the 1-Wire bus.
  */
uint8_t one_wire_read_bit(void) 
{
    uint8_t bit;
    LPC_GPIO0->FIODIR |= (1 << ONE_WIRE_PIN); // Setting pin as output
    LPC_GPIO0->FIOCLR = (1 << ONE_WIRE_PIN);  // Setting pin as low
    delay_us(3);                              // Delay 3 microseconds
	
    LPC_GPIO0->FIODIR &= ~(1 << ONE_WIRE_PIN); // Setting pin as input
    delay_us(10);                             // Delay 10 microseconds
	
    bit = (LPC_GPIO0->FIOPIN & (1 << ONE_WIRE_PIN)) != 0; // Reading bit send by slave
    delay_us(65);                             // Delay 65 microseconds
	
    return bit;
}

/** Sends a byte on the 1-Wire bus.
  */
void one_wire_write_byte(uint8_t byte) 
{
    for (int i = 0; i < 8; i++) 
	{
        one_wire_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

/** Reads a byte from the 1-Wire bus.
  */
uint8_t one_wire_read_byte(void)
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++)
	{
        byte |= (one_wire_read_bit() << i);
    }
    return byte;
}


/** Reads a serial number (64 bits) of device.
  * Returns 0 if everything is OK.
  * Returns -1 if there is no slave present.
  * Returns -2 if checksum is incorrect.
  */
int read_serial_number(uint8_t serial_number[]) 
{
	//send_UART_string("reseting \n\r");
	if(one_wire_reset())
	{
		//send_UART_string("send read ROM\n\r");
		 one_wire_write_byte(0x33); // "Read ROM" command
		//send_UART_string("reading...\n");
		for (int i = 0; i < 8; i++) 
		{
			serial_number[i] = one_wire_read_byte();
		}

		
		if (check_crc(serial_number)) 
		{
			// CRC is correct
			return 0;
		} else 
		{
			// CRC is incorrect
			return -2;
		}
	}
	else
	{
		// slave is not present
		return -1;
	}	
}

