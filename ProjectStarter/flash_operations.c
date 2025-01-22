#include "flash_operations.h"
#include "uart0.h"
#include <string.h>
#include <stdio.h>

#define IAP_LOCATION 0x1FFF1FF1
typedef void (*IAP)(unsigned int[], unsigned int[]);
IAP iap_entry = (IAP) IAP_LOCATION;

#define SYSTEM_CLOCK_KHZ (SystemCoreClock / 1000)

//Funkcja wysylajaca na uart0 tekstowe rozwiniecie bledu IAP
void send_error_uart(unsigned int n);
// Funkcja wyliczajaca adres sektora Flash na podstawie jego numeru
uint32_t get_flash_sector_address(uint32_t sector_number);
//funkcja przygotowujaca sektor pamieci Flash
bool prepare_sector(uint32_t sector_number);
//Funkcja czyszczaca sektor pamieci Flash
bool erase_sector(uint32_t sector_number);
// Funkcja zapisu danych do sektora pamieci Flash
bool write_to_flash_sector(uint32_t sector_number, uint8_t *data, uint32_t size);
// Funkcja odczytu danych z pamieci Flash
bool read_from_flash(uint32_t sector_number, uint8_t *buffer, uint32_t size, uint32_t offset);
//Funkcja ustawiajaca zapis sygnalizujacy ze juz istnieja dane w pamieci Flash
int set_code(uint8_t* code);
//Funkcja czyszczaca sektory przy inicjalizacji
int clear_sectors(void);
//Funkcja zwracająca ilosc zarejestrowanych pastylek
uint8_t get_number_of_registered(void);
//Funkcja ustawiajaca ilosc zarejestrowanych pastylek (0 success, 1 blad memory allocation, -1 blad flash)
int set_number_of_registered(uint8_t new_number);
//Funkcja zwracająca ilosc wpisow w historii
uint16_t get_history_entries(void);
//Funkcja ustawiajaca ilosc wpisow w historii
int set_history_entries(uint16_t new_number);
int add_history(uint8_t serial_number[], uint8_t date[]);
int print_history();

// Test dziala poprawnie, jesli on dziala to znaczy ze jest dobrze
void flash_test(void)
{
  uint32_t sector_number = 16;
	
	uint8_t *data_to_write = (uint8_t*)malloc(sizeof(uint8_t) * 256);
	char* zosia = "zosia";
	for(int i = 0; zosia[i] != '\0'; i++)
	{
		data_to_write[i] = zosia[i];
		data_to_write[i+1] = '\0';
	}
	write_to_flash_sector(sector_number, data_to_write, 256);
	
	uint8_t *read_buffer = (uint8_t*)malloc(sizeof(uint8_t) * 256);
	read_from_flash(sector_number, read_buffer, 256, 0);
	send_UART_string((char*)read_buffer);
	
	free(data_to_write);
	free(read_buffer);
}

int initialize_flash(void)
{
		__disable_irq();
    uint8_t code[8];
    uint8_t password[8] = {1,2,4,2,5,4,2,7};

    if (!read_from_flash(MAINTANANCE_REGISTER, code, sizeof(code), 0)) 
    {
        send_UART_string("flash init: read flash failed\n");
       __enable_irq();
        return -1; // Reading from flash failed
    }
    
    if (memcmp(code, password, 8) == 0){
        send_UART_string("flash init: code present\n");
        __enable_irq();
        return 0;
    }
    prepare_sector(BUTTON_REGISTER);
    erase_sector(BUTTON_REGISTER);
    prepare_sector(MAINTANANCE_REGISTER);
    erase_sector(MAINTANANCE_REGISTER);
    prepare_sector(HISTORY_REGISTER);
    erase_sector(HISTORY_REGISTER);

    set_number_of_registered(0);
    set_history_entries(0);
    set_code(password);
		
    send_UART_string("flash init: code added\n");
	__enable_irq();
    return 0;
}


bool is_registered(uint8_t serial_number[]) 
{
    __disable_irq();
    uint8_t read_number[8];

    uint8_t saved = get_number_of_registered(); // Read from flash the number of saved entries
    if (saved == (uint8_t)-1)
    {
        __enable_irq();
        return false; // Maximum limit reached or error in reading
    }
    for (uint8_t i = 0; i < saved; i++)
    {
        // Read 8-byte serial number from flash for the current index
        if (!read_from_flash(BUTTON_REGISTER, read_number, 8, i * 8))
        {
            send_UART_string("is_reg: read flash failed\n");
            break; // Stop if reading fails
        }

        // Compare the read number with the provided serial number
        if (memcmp(serial_number, read_number, 8) == 0)
        {
            __enable_irq();
            send_UART_string("is_reg: found\n");
            return true; // Match found
        }
    }

		send_UART_string("is_reg:not found\n");
    __enable_irq();
    return false; // Not found
}

int add_history(uint8_t serial_number[], uint8_t date[])
{
	__disable_irq();

    uint16_t saved = get_history_entries(); // Read from flash the number of saved entries
    if (saved == (uint16_t)-1)
    {
        __enable_irq();
        return 1;
    }
		uint16_t data_size = sizeof(uint8_t) * 16 * 256;
    uint8_t *data = (uint8_t *)malloc(data_size);
    if (data == NULL)
    {
        send_UART_string("add_his: mem failed\n");
        __enable_irq();
        free(data);
        return 1; // Memory allocation failed
    }
    
    if (!read_from_flash(HISTORY_REGISTER, data, data_size, 0))
    {
        send_UART_string("add_his: read flash failed\n");
        __enable_irq();
        free(data);
        return -1; // Failed to read from flash
    }

    if (saved >= 256)
    {
        for (int i = 0; i < 256 - 1; i++){
            memcpy(data + i * 16, data + (i + 1) * 16, 16);
        }
        
        memcpy(data + 255 * 16, serial_number, 8);
        memcpy(data + 255 * 16 + 8, date, 6);
    }
    else
    {
        memcpy(data + saved * 16, serial_number, 8);
        memcpy(data + saved * 16 + 8, date, 6);
    }

    if (!write_to_flash_sector(BUTTON_REGISTER, data, data_size))
    {
        send_UART_string("add_his: write flash failed\n");
        __enable_irq();
        free(data);
        return -1; // Failed to write to flash
    }
		
		char buff[32];
		sprintf(buff, "ad_his:before:%d", saved);
		send_UART_string(buff);

    saved++;
    if (set_history_entries(saved) != 0)
    {
        __enable_irq();
        free(data);
        return -1; // Failed to update the count of registered numbers
    }
		saved = get_history_entries();
		sprintf(buff, "ad_his:after:%d", saved);
		send_UART_string(buff);
    send_UART_string("add_his: success\n");
    __enable_irq();
    free(data);
    return 0; // Success
}

int add_iButton(uint8_t serial_number[])
{
		__disable_irq();
    uint8_t saved = get_number_of_registered(); // Read from flash the number of saved entries
		
    if (saved >= 32 || saved == (uint8_t)-1)
    {
        __enable_irq();
				send_UART_string("add_iB: max_num_reg\n");
        return 1; // Maximum limit reached or error in reading
    }
    
		uint16_t data_size = sizeof(uint8_t) * 8 * 32;
    uint8_t *data = (uint8_t *)malloc(data_size);
    if (data == NULL)
    {
        send_UART_string("add_iB:mem failed\n");
        __enable_irq();
        return 1; // Memory allocation failed
    }
    
    if (!read_from_flash(BUTTON_REGISTER, data, data_size, 0))
    {
        send_UART_string("add_iB:read flash failed\n");
        __enable_irq();
        free(data);
        return -1; // Failed to read from flash
    }

    memcpy(data + saved * 8, serial_number, 8); // Copy serial number to the appropriate location

    if (!write_to_flash_sector(BUTTON_REGISTER, data, data_size))
    {
        send_UART_string("add_iB:write flash failed\n");
        __enable_irq();
        free(data);
        return -1; // Failed to write to flash
    }
    saved++;
    if (set_number_of_registered(saved) != 0)
    {
        __enable_irq();
        free(data);
		send_UART_string("add_iB:failed to update count\n");
        return -1; // Failed to update the count of registered numbers
    }
    send_UART_string("add_iB:success\n");
    free(data);
		__enable_irq();
    return 0; // Success
}

int print_history()
{
    __disable_irq();
    send_UART_string("print_his: ");

    uint8_t data[15];

    uint16_t saved = get_history_entries(); // Read from flash the number of saved entries
    if(saved > 256)
       saved=256;
    
    if (saved == (uint16_t)-1) {  // Error reading
        __enable_irq();
        return -1;
    }

    for (int i = 0; i < saved; i++){
        if (!read_from_flash(HISTORY_REGISTER, data, 14, i * 16))
        {
            send_UART_string("read flash failed\n");
            break; // Stop if reading fails
        }
		data[14] = '\0';
        send_UART_string((char*)data);
        send_UART_string("\n");
    }

    send_UART_string("success\n");
    __enable_irq();
    return false; // Not found
}

int delete_iButton(uint8_t serial_number[])
{
    __disable_irq();
    uint8_t saved = get_number_of_registered();
	
    if (saved == (uint8_t)-1) {  // Error reading
		send_UART_string("del_iB:get_reg_err\n");
        __enable_irq();
        return -1;
    }
    if(saved == 0) {
        send_UART_string("del_iB:no buttons registered\n");
        __enable_irq();
        return 1;  // Not found
    }
    if(!is_registered(serial_number)){
        send_UART_string("del_iB:iB not registered\n");
        __enable_irq();
        return 1;  // Not found
    }

    uint8_t *data = malloc(sizeof(uint8_t) * 8 * 32);
    if (data == NULL) {
        send_UART_string("del_iB:mem failed\n");
        __enable_irq();
        return -1;
    }
    if (!read_from_flash(BUTTON_REGISTER, data, sizeof(uint8_t) * 8 * 32, 0))
    {
        __enable_irq();
        free(data);
        send_UART_string("del_iB:read flash failed\n");
        return -1;
    }
		
    char buff[20];

    bool found = false;
    for (uint8_t i = 0; i < saved; i++) {
			
        sprintf(buff, "%d,%d,%d,%d,%d,%d,%d,%d\n", data[i * 8 + 0],data[i * 8 + 1],data[i * 8 + 2],data[i * 8 + 3],data[i * 8 + 4],data[i * 8 + 5],data[i * 8 + 6],data[i * 8 + 7]);
        send_UART_string(buff);
        
        if (!found && (memcmp(data + i * 8, serial_number, 8) == 0)) {
            found = true;  // Mark as removed
            if(i<31)
               memcpy(data + i * 8, data + (i+1) * 8, (31-i) * 8); // shift all records
            memset(data + (saved - 1) * 8, 0xFF, (32-(saved-1)) * 8); // clear rest of buffer
			   break;
        }
    }

    if (!found) {
        __enable_irq();
        free(data);
        send_UART_string("del_iB:found not removed\n");
        return 1;
    }

    if (!write_to_flash_sector(BUTTON_REGISTER, data, sizeof(uint8_t) * 8 * 32))
    {
        __enable_irq();
        free(data);
        send_UART_string("del_iB:write flash failed\n");
        return -1;
    }//w tym ifie wywala
		send_UART_string("hi\n");
    
	free(data);
    set_number_of_registered(saved - 1);
    send_UART_string("del_iB:success\n");
    __enable_irq();
    return 0;
}

bool prepare_sector(uint32_t sector_number)
{
	send_UART_string("p1");
    unsigned int command[5];
    unsigned int result[5];
    command[0] = 50;  // Prepare Sector
    command[1] = sector_number;  // Numer sektora
    command[2] = sector_number;  // Numer sektora (tylko jeden sektor)
    iap_entry(command, result);
	//miedzy tymi blad przy delete tak ze apke wywala (sprobuje zmienic sektor)
	send_UART_string("p2");
    if (result[0] != 0)
    {
			send_error_uart(result[0]);
        return false;  // Obsluga bledu
    }
		
		send_UART_string("p3");
    return true; // Sukces
}

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
		send_error_uart(result[0]);
        return false;
    }
    return true; // Sukces
}


bool read_from_flash(uint32_t sector_number, uint8_t *buffer, uint32_t size, uint32_t offset)
{
    // Oblicz adres startowy sektora
    uint32_t flash_address = get_flash_sector_address(sector_number);
    if (flash_address == 0xFFFFFFFF)
    {
        send_UART_string("read flash: wrong sector\n");
        // Blad: nieprawidlowy numer sektora
        return false;
    }
		
    for (uint32_t i = 0; i < size; i++)
    { // Kopiowanie danych z pamieci Flash do bufora
        buffer[i] = *((volatile uint8_t *)(flash_address + i + offset));
    }
    return true;  // Sukces
}

bool write_to_flash_sector(uint32_t sector_number, uint8_t *data, uint32_t size)
{
    unsigned int command[5];
    unsigned int result[5];
    
    uint32_t flash_address = get_flash_sector_address(sector_number); // Oblicz adres sektora
    if (flash_address == 0xFFFFFFFF)
    {
        // Blad: nieprawidlowy numer sektora
        send_UART_string("write flash: wrong sector\n");
        return false;
    }
		
	if (size > 4096 || size % 256 != 0)
    {
        send_UART_string("write flash: data size invalid.\n");
        return false;
    }
		send_UART_string("1\n");
    if (!prepare_sector(sector_number)) return false; // Przygotowanie sektora do zapisu
    if (!erase_sector(sector_number)) return false; // Czyszczenie sektora przez zapisam(musi byc)
    if (!prepare_sector(sector_number)) return false; // Przygotowanie sektora do zapisu
		//pomiedzy tymi dwoma blad przey delete
		send_UART_string("2\n");
    // Zapis danych
    command[0] = 51;  // Copy RAM to Flash
    command[1] = flash_address;  // Adres docelowy w pamieci Flash
    command[2] = (unsigned int)data;  // Dane w RAM
    command[3] = size;  // Liczba bajt�w do zapisania
    command[4] = SYSTEM_CLOCK_KHZ;  // Czestotliwosc zegara
    iap_entry(command, result);
		send_UART_string("3\n");
    if (result[0] != 0)
	{
	    send_error_uart(result[0]);
        return false;  // Obsluga bledu
    }
    return true;  // Sukces
}

uint8_t get_number_of_registered(void)
{
    uint8_t data[1];
    if (!read_from_flash(MAINTANANCE_REGISTER, data, sizeof(data), R_NUM_OFFSET)) 
    {
        send_UART_string("get_nr_reg: read flash failed\n");
        return -1;
    }
    return data[0];
}

int set_number_of_registered(uint8_t new_number)
{
		uint8_t *data = (uint8_t*)malloc(sizeof(uint8_t) * 8 * 32);
	
    if (data == NULL)
    {
        send_UART_string("set_nr_reg: mem failed\n");
        return 1;
    }

    if (!read_from_flash(MAINTANANCE_REGISTER, data, sizeof(uint8_t) * 8 * 32, 0)) 
    {
        free(data);
        send_UART_string("set_nr_reg: read flash failed\n");
        return -1;
    }
		
    data[R_NUM_OFFSET] = new_number;
		
    if(!write_to_flash_sector(MAINTANANCE_REGISTER, data, sizeof(uint8_t) * 8 * 32)) 
    {
        free(data);
        send_UART_string("set_nr_reg: write flash failed\n");
        return -1;
    }
		free(data);
		send_UART_string("set_nr_reg: success\n");
		free(data);
    return 0;
}

int set_code(uint8_t* code)
{
    send_UART_string("set_code: ");
    uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t) * 8  * 32); // Allocate heap memory
    if (data == NULL)
    {
        send_UART_string("mem failed\n");
        return 1; // Memory allocation failed
    }

    if (!read_from_flash(MAINTANANCE_REGISTER, data, 8 * 32, 0)) 
    {
        free(data);
        send_UART_string("read flash failed\n");
        return -1; // Reading from flash failed
    }

    memcpy(data, code, 8);

    if (!write_to_flash_sector(MAINTANANCE_REGISTER, data, 8 * 32)) 
    {
        free(data);
        send_UART_string("write flash failed\n");
        return -1; // Writing to flash failed
    }

    free(data);
    send_UART_string("success\n");
    return 0;
}

uint16_t get_history_entries(void)
{
    uint8_t data[2];

    if (!read_from_flash(MAINTANANCE_REGISTER, data, sizeof(data), H_NUM_OFFSET)) 
    {
        send_UART_string("get_hist_ent: read flash failed\n");
        return -1; // Reading from flash failed
    }
		uint16_t result = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
    char buff[32];
    sprintf(buff, "get_hist_ent: success: value=%d\n", result);
    send_UART_string(buff);
    return result;
}

int set_history_entries(uint16_t new_number)
{
    uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t) * 8 * 32); // Allocate heap memory
    if (data == NULL)
    {
        send_UART_string("set_hist_ent: mem failed\n");
        return 1; // Memory allocation failed
    }

    if (!read_from_flash(MAINTANANCE_REGISTER, data, sizeof(uint8_t) * 8 * 32, 0)) 
    {
        free(data);
        send_UART_string("set_hist_ent: read flash failed\n");
        return -1; // Reading from flash failed
    }

    // Update the uint16_t value at the appropriate offset
    data[H_NUM_OFFSET] = (uint8_t)(new_number & 0xFF);
    data[H_NUM_OFFSET + 1] = (uint8_t)((new_number >> 8) & 0xFF);
		
    if (!write_to_flash_sector(MAINTANANCE_REGISTER, data, sizeof(uint8_t) * 8 * 32)) 
    {
        free(data);
        send_UART_string("set_hist_ent: write flash failed\n");
        return -1; // Writing to flash failed
    }

    free(data);
    send_UART_string("set_hist_ent: success\n");
    return 0;
}

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

void send_error_uart(unsigned int n)
{
    switch (n)
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
