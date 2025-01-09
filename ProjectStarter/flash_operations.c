#include "flash_operations.h"
#include "uart0.h"
#include <string.h>
#include <stdio.h>

#define IAP_LOCATION 0x1FFF1FF1
typedef void (*IAP)(unsigned int[], unsigned int[]);
IAP iap_entry = (IAP) IAP_LOCATION;

#define SYSTEM_CLOCK_KHZ (SystemCoreClock / 1000);

// Funkcja wyliczajaca adres sektora Flash na podstawie jego numeru
uint32_t get_flash_sector_address(uint32_t sector_number)
{
    if (sector_number < 16) {// Sektory 0-15 (4 KB kazdy)
        return sector_number * 4 * 1024;  // 4 KB = 4096 bajt�w
    } else if (sector_number < 30) {// Sektory 16-29 (32 KB kazdy)
        return 64 * 1024 + (sector_number - 16) * 32 * 1024;  // 64 KB offset + 32 KB sektory
    } else {// Blad: sektor poza zakresem
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
    if (result[0] != 0) {
				send_UART_string("Failed to prepare\n");
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
    if (result[0] != 0) {
				send_UART_string("Failed to erase\n");
        return false;  // Obsluga bledu
    }
		send_UART_string("erase success\n");
    return true; // Sukces
}

// Funkcja sprawdzajaca, czy sektor Flash jest w calosci pusty
bool is_sector_empty(uint32_t sector_number)
{
    uint32_t flash_address = get_flash_sector_address(sector_number); // Oblicz adres startowy sektora
    if (flash_address == 0xFFFFFFFF) {
        // Blad: nieprawidlowy numer sektora
        return false;
    }
    uint32_t sector_size = (sector_number < 16) ? 4 * 1024 : 32 * 1024; // Oblicz rozmiar sektora (4 KB dla sektor�w 0�15, 32 KB dla sektor�w 16�29)

    for (uint32_t i = 0; i < sector_size; i++) { // Iteracja przez sektor w poszukiwaniu danych innych niz 0xFF
        if (*((volatile uint8_t *)(flash_address + i)) != 0xFF) {
            return false;  // Znalazl zajety bajt
        }
    }
    return true;  // Sektor jest pusty
}

// Funkcja zapisu danych do sektora pamieci Flash
bool write_to_flash_sector(uint32_t sector_number, uint8_t *data, uint32_t size)
{
    unsigned int command[5];
    unsigned int result[5];
    
    uint32_t flash_address = get_flash_sector_address(sector_number); // Oblicz adres sektora
    if (flash_address == 0xFFFFFFFF) {
        // Blad: nieprawidlowy numer sektora
        return false;
    }
		
		if (size % 256 != 0) {
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
				//char buffer[24];
				//sprintf(buffer, "%d", result[0]);
				//send_UART_string(buffer);
				send_UART_string("Failed to write\n");
        return false;  // Obsluga bledu
    }
		send_UART_string("write success\n");
    return true;  // Sukces
}

// Funkcja odczytu danych z pamieci Flash
bool read_from_flash(uint32_t sector_number, uint8_t *buffer, uint32_t size, uint32_t offset = 0)
{
    // Oblicz adres startowy sektora
    uint32_t flash_address = get_flash_sector_address(sector_number);
    if (flash_address == 0xFFFFFFFF) {
        // Blad: nieprawidlowy numer sektora
        return false;
    }
		
    for (uint32_t i = 0; i < size; i++) { // Kopiowanie danych z pamieci Flash do bufora
        buffer[i] = *((volatile uint8_t *)(flash_address + i));
    }
    return true;  // Sukces
}

// Funkcja por�wnujaca dane (zapisane i odczytane)
bool verify_flash_data(uint8_t *data, uint8_t *buffer, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        if (data[i] != buffer[i]) {
            //send_UART_string("Verification failed\n");
            return false;
        }
    }
    //send_UART_string("verified successfully\n");
    return true;
}



bool is_registered(uint8_t serial_number[]) 
{
    uint8_t *read_number = (uint8_t *)malloc(8 * sizeof(uint8_t));
    if (read_number == NULL) {
        // Obsługa błędu alokacji pamięci
        return false;
    }

    for (int i = 0; i < 32; i++) { // Baza danych zawiera 32 numery po 8 bajtów
        read_from_flash(DATABASE_SECTOR, read_number, 8, i * 8);
        if (verify_flash_data(serial_number, read_number, 8)) {
            free(read_number);
            return true;
        }
    }
    
    free(read_number);
    return false;
}

void add_history(uint8_t serial_number[], uint8_t date[])
{
}

int add_iButton(uint8_t serial_number[])
{
    return 0;
}


void print_history()
{
}

int delete_iButton(uint8_t serial_number[])
{
    return 0;
}