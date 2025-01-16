#include "flash_operations.h"
#include "uart0.h"
#include <string.h>
#include <stdio.h>

#define IAP_LOCATION 0x1FFF1FF1
typedef void (*IAP)(unsigned int[], unsigned int[]);
IAP iap_entry = (IAP) IAP_LOCATION;

#define SYSTEM_CLOCK_KHZ (SystemCoreClock / 1000)

void send_error_uart0(unsigned int *n)
{
    switch (*n)
    {
    case 1:
        send_UART_string("INVALID_COMMAND\n");
        break;
    case 2:
        send_UART_string("SRC_ADDR_ERROR\n");
        break;
    case 3:
        send_UART_string("DST_ADDR_ERROR\n");
        break;
    case 4:
        send_UART_string("SRC_ADDR_NOT_MAPPED\n");
        break;
    case 5:
        send_UART_string("DST_ADDR_NOT_MAPPED\n");
        break;
    case 6:
        send_UART_string("COUNT_ERROR\n");
        break;
    case 7:
        send_UART_string("INVALID_SECTOR\n");
        break;
    case 8:
        send_UART_string("SECTOR_NOT_BLANK\n");
        break;
    case 9:
        send_UART_string(" SECTOR_NOT_PREPARED\n");
        break;
    case 10:
        send_UART_string("COMPARE_ERROR\n");
        break;
    case 11:
        send_UART_string("BUSY\n");
        break;
    default:
        send_UART_string("UNKNOWN_ERROR\n");
        break;
    }
}

// Funkcja wyliczajaca adres sektora Flash na podstawie jego numeru
uint32_t get_flash_sector_address(uint32_t sector_number)
{
    if (sector_number < 16)
    {// Sektory 0-15 (4 KB kazdy)
        return sector_number * 4 * 1024;  // 4 KB = 4096 bajt�w
    } 
    else if (sector_number < 30)
    {// Sektory 16-29 (32 KB kazdy)
        return 64 * 1024 + (sector_number - 16) * 32 * 1024;  // 64 KB offset + 32 KB sektory
    } 
    else
    {// Blad: sektor poza zakresem
        return 0xFFFFFFFF;  // Nieprawidlowy adres
    }
}

//funkcja przygotowujaca sektor pamieci Flash
bool prepare_sector(uint32_t sector_number)
{
    unsigned int command[5];
    unsigned int result[5];
    command[0] = 50;  // Prepare Sector
    command[1] = sector_number;  // Numer sektora
    command[2] = sector_number;  // Numer sektora (tylko jeden sektor)
    iap_entry(command, result);
    if (result[0] != 0)
    {
			send_error_uart0(result);
        return false;  // Obsluga bledu
    }
		send_UART_string("prepare success\n");
    return true; // Sukces
}

//Funkcja czyszczaca sektor pamieci Flash
bool erase_sector(uint32_t sector_number)
{
    unsigned int command[5];
    unsigned int result[5];
    command[0] = 52;  // Erase Sector
    command[1] = sector_number;  // Numer sektora
    command[2] = sector_number;  // Numer sektora (tylko jeden sektor
    command[3] = SYSTEM_CLOCK_KHZ; // Czestotliwosc zegara w kHz
    iap_entry(command, result);
    if (result[0] != 0)
    {
		send_error_uart0(result);
        return false;
    }
		send_UART_string("erase success\n");
    return true; // Sukces
}

// Funkcja zapisu danych do sektora pamieci Flash
bool write_to_flash_sector(uint32_t sector_number, uint8_t *data, uint32_t size)
{
    unsigned int command[5];
    unsigned int result[5];
    
    uint32_t flash_address = get_flash_sector_address(sector_number); // Oblicz adres sektora
    if (flash_address == 0xFFFFFFFF)
    {
        // Blad: nieprawidlowy numer sektora
        return false;
    }
		
		if (size % 256 != 0)
        {
        send_UART_string("Error: Data size must be a multiple of 256 bytes.\n");
        return false;
    }
		
    if (!prepare_sector(sector_number)) return false; // Przygotowanie sektora do zapisu
    if (!erase_sector(sector_number)) return false; // Czyszczenie sektora przez zapisam(musi byc)
    if (!prepare_sector(sector_number)) return false; // Przygotowanie sektora do zapisu
		
    // Zapis danych
    command[0] = 51;  // Copy RAM to Flash
    command[1] = flash_address;  // Adres docelowy w pamieci Flash
    command[2] = (unsigned int)data;  // Dane w RAM
    command[3] = size;  // Liczba bajt�w do zapisania
    command[4] = SYSTEM_CLOCK_KHZ;  // Czestotliwosc zegara
    iap_entry(command, result);
    if (result[0] != 0)
	{
		send_error_uart0(result);
      return false;  // Obsluga bledu
    }
		send_UART_string("write success\n");
    return true;  // Sukces
}

// Funkcja odczytu danych z pamieci Flash
bool read_from_flash(uint32_t sector_number, uint8_t *buffer, uint32_t size, uint32_t offset)
{
    // Oblicz adres startowy sektora
    uint32_t flash_address = get_flash_sector_address(sector_number);
    if (flash_address == 0xFFFFFFFF)
    {
        // Blad: nieprawidlowy numer sektora
        return false;
    }
		
    for (uint32_t i = 0; i < size; i++)
    { // Kopiowanie danych z pamieci Flash do bufora
        buffer[i] = *((volatile uint8_t *)(flash_address + i));
    }
    return true;  // Sukces
}

uint8_t get_number_of_registered(void)
{
	uint8_t *data = (uint8_t*)malloc(sizeof(uint8_t));
    if (data == NULL)
    {
        return -1; // Memory allocation failed
    }

    if (!read_from_flash(MAINTANANCE_REGISTER, data, sizeof(uint8_t), R_NUM_OFFSET)) 
    {
        free(data); // Free allocated memory
        return -1; // Reading from flash failed
    }

    return *data;
}

int set_number_of_registered(uint8_t new_number)
{
	uint8_t *data = (uint8_t*)malloc(sizeof(uint8_t) * 8 * 32);
    if (data == NULL)
    {
        return 1;
    }

    if (!read_from_flash(MAINTANANCE_REGISTER, data, sizeof(uint8_t) * 8 * 32, 0)) 
    {
        free(data);
        return -1;
    }

    data[R_NUM_OFFSET] = new_number;
    if(!write_to_flash_sector(MAINTANANCE_REGISTER, data, 8*32)) 
    {
        free(data);
        return -1;
    }
    free(data);
    return 0;
}

uint16_t get_history_entries(void)
{
    uint8_t *data = (uint8_t *)malloc(sizeof(uint16_t));
    if (data == NULL)
    {
        return -1; // Memory allocation failed
    }

    if (!read_from_flash(MAINTANANCE_REGISTER, data, sizeof(uint16_t), H_NUM_OFFSET)) 
    {
        free(data);
        return -1; // Reading from flash failed
    }

    uint16_t result = *((uint16_t *)data); // Convert the buffer data to uint16_t
    free(data); // Free allocated memory
    return result;
}

int set_history_entries(uint16_t new_number)
{
    uint8_t *data = (uint8_t *)malloc(8 * 32); // Allocate heap memory
    if (data == NULL)
    {
        return 1; // Memory allocation failed
    }

    if (!read_from_flash(MAINTANANCE_REGISTER, data, 8 * 32, 0)) 
    {
        free(data);
        return -1; // Reading from flash failed
    }

    // Update the uint16_t value at the appropriate offset
    *((uint16_t *)(data + H_NUM_OFFSET)) = new_number;

    if (!write_to_flash_sector(MAINTANANCE_REGISTER, data, 8 * 32)) 
    {
        free(data);
        return -1; // Writing to flash failed
    }

    free(data);
    return 0;
}

bool is_registered(uint8_t serial_number[]) 
{
    __disable_irq();

    uint8_t *read_number = (uint8_t *)malloc(8 * sizeof(uint8_t));
    if (read_number == NULL)
    {
        __enable_irq();
        return false; // Memory allocation failed
    }

    uint8_t saved = get_number_of_registered(); // Read from flash the number of saved entries
    for (uint8_t i = 0; i < saved; i++)
    {
        // Read 8-byte serial number from flash for the current index
        if (!read_from_flash(BUTTON_REGISTER, read_number, 8, i * 8))
        {
            break; // Stop if reading fails
        }

        // Compare the read number with the provided serial number
        if (memcmp(serial_number, read_number, 8) == 0)
        {
            free(read_number);
            __enable_irq();
            return true; // Match found
        }
    }

    free(read_number);
    __enable_irq();
    return false; // Not found
}

void add_history(uint8_t serial_number[], uint8_t date[])
{
		__disable_irq();

		__enable_irq();
}

int add_iButton(uint8_t serial_number[])
{
	__disable_irq();

    uint8_t saved = get_number_of_registered(); // Read from flash the number of saved entries
    if (saved >= 32 || saved == (uint8_t)-1)
    {
        __enable_irq();
        return 1; // Maximum limit reached or error in reading
    }
    
	uint16_t data_size = sizeof(uint8_t) * 8 * 32;
    uint8_t *data = (uint8_t *)malloc(data_size);
    if (data == NULL)
    {
        __enable_irq();
        return 1; // Memory allocation failed
    }
    
    if (!read_from_flash(BUTTON_REGISTER, data, data_size, 0))
    {
        free(data);
        __enable_irq();
        return -1; // Failed to read from flash
    }

    memcpy(&data[saved * 8], serial_number, 8); // Copy serial number to the appropriate location

    if (!write_to_flash_sector(BUTTON_REGISTER, data, data_size))
    {
        free(data);
        __enable_irq();
        return -1; // Failed to write to flash
    }

    saved++;
    if (set_number_of_registered(saved) != 0)
    {
        free(data);
        __enable_irq();
        return -1; // Failed to update the count of registered numbers
    }
    free(data);
    __enable_irq();
    return 0; // Success
}


void print_history()
{
		__disable_irq();
		__enable_irq();
}

int delete_iButton(uint8_t serial_number[])
{
    __disable_irq();

    uint8_t saved = get_number_of_registered();
    if (saved == (uint8_t)-1) {  // Error reading
        __enable_irq();
        return -1;
    }
    if (saved == 0 || !is_registered(serial_number)) {
        __enable_irq();
        return 1;  // Not found
    }

    uint8_t *data = malloc(8 * 32);
    if (!data) {
        __enable_irq();
        return -1;
    }

    if (!read_from_flash(BUTTON_REGISTER, data, 8 * 32, 0)) {
        free(data);
        __enable_irq();
        return -1;
    }

    bool found = false;
    for (uint8_t i = 0; i < saved; i++) {
        if (!found && if (memcmp(data + i * 8, serial_number, 8) == 0)) {
            found = true;  // Mark as removed
        }
        if (found && i < saved - 1) {
            // Shift data to fill the gap
            memcpy(data + i * 8, data + (i + 1) * 8, 8);
        }
    }

    if (!found) {
        free(data);
        __enable_irq();
        return 1;
    }

    // Clear the last entry after shifting
    memset(data + (saved - 1) * 8, 0xFF, 8);

    if (!write_to_flash_sector(BUTTON_REGISTER, data, 8 * 32)) {
        free(data);
        __enable_irq();
        return -1;
    }
    
	free(data);
    set_number_of_registered(saved - 1);
    __enable_irq();
    return 0;
}

void initialize_flash()
{
		__disable_irq();
		__enable_irq();
}