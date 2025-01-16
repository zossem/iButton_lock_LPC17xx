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

// Funkcja por�wnujaca dane
bool verify_data(uint8_t *data, uint8_t *buffer, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
    {
        if (data[i] != buffer[i])
        {
            return false;
        }
    }
    return true;
}

uint8_t get_number_of_registered(void)
{
	uint8_t *data = (uint8_t*)malloc(sizeof(uint8_t));
    if (!read_from_flash(MAINTANANCE_REGISTER, data, sizeof(uint8_t), R_NUM_OFFSET)) 
    {
        free(data);
        return -1;
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
		__enable_irq();
        return -1;
    }
    data[R_NUM_OFFSET] = new_number;
    if(!write_to_flash_sector(MAINTANANCE_REGISTER, data, 8*32)) 
    {
        free(data);
        return -1;
    }
    free(data);
}

bool is_registered(uint8_t serial_number[]) 
{
	__disable_irq();
    uint8_t *read_number = (uint8_t *)malloc(8 * sizeof(uint8_t));
    if (read_number == NULL)
    {
		__enable_irq();
        return false;
    }
    uint8_t saved = get_number_of_registered(); //read from flash ilosc zapisanych
    for (uint8_t i = 0; i < saved; i++)
    { // Baza danych zawiera 32 numery po 8 bajtów
        if(!read_from_flash(BUTTON_REGISTER, read_number, 8, i * 8)) break;

        if (verify_data(serial_number, read_number, 8))
        {
            free(read_number);
			__enable_irq();
            return true;
        }
    }
    free(read_number);
		__enable_irq();
    return false;
}

void add_history(uint8_t serial_number[], uint8_t date[])
{
		__disable_irq();
		__enable_irq();
}

int add_iButton(uint8_t serial_number[])
{
	__disable_irq();
    uint8_t saved = get_number_of_registered(); //read from flash ilosc zapisanych
    if (saved >= 32 || saved == -1)
    {
		__enable_irq();
        return 1;
    }
	uint8_t *data = (uint8_t*)malloc(sizeof(uint8_t) * 8 * 32);
    if (data == NULL)
    {
		__enable_irq();
        return false;
    }
    if (!read_from_flash(BUTTON_REGISTER, data, sizeof(uint8_t) * 8 * 32, 0)) 
    {
        free(data);
				__enable_irq();
        return -1;
    }
    for (uint8_t i = 0; i < 8; i++)
    {
        data[saved * 8 + i] = serial_number[i];
    }
    if(!write_to_flash_sector(BUTTON_REGISTER, data, 8*32)) 
    {
        free(data);
				__enable_irq();
        return -1;
    }
    saved++;
    set_number_of_registered(saved)
    free(data);
	__enable_irq();
    return 0;
}


void print_history()
{
		__disable_irq();
		__enable_irq();
}

int delete_iButton(uint8_t serial_number[])
{
    __disable_irq();
    if (is_registered(serial_number) == false)
		{
			__enable_irq();
			return 1;
		}
    uint8_t saved = 1; //read from flash ilosc zapisanych
    bool removed = false;
	uint8_t *data = (uint8_t*)malloc(sizeof(uint8_t) * 8 * 32);
    if (!read_from_flash(BUTTON_REGISTER, data, sizeof(uint8_t) * 8 * 32, 0)) 
    {
				__enable_irq();
        free(data);
        return -1;
    }
    uint8_t count = 0;
    while (!removed)
    {
        if(verify_data(data + count * 8, serial_number, 8))
        {
            removed = true;
            for (uint8_t i = 0; i < 8; i++)
            {
                data[count + i] = 0xFF;
            }
        }
        count++;
    }

    for (uint8_t i = count; i < saved - 1; i++)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            data[i] = data[i+8];
        }
    }

    if(!write_to_flash_sector(BUTTON_REGISTER, data, 8*32)) 
    {
        free(data);
				__enable_irq();
        return -1;
    }
    
		__enable_irq();
    saved--; //zapisac do flash
    return 0;
}

void initialize_flash()
{
		__disable_irq();
		__enable_irq();
}