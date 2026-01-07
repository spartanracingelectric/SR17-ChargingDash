#ifndef EEPROM_H
#define EEPROM

#include "main.h"
#include "stm32f1xx_hal_def.h"

#define EEPROM_READ 0b00000011
#define EEPROM_WRITE 0b0000010
#define EEPROM_WRDI 0b00000100
#define EEPROM_WREN 0b00000110
#define EEPROM_RDSR 0b00000101
#define EEPROM_WRSR 0b00000001

#define EEPROM_SIZE_BYTES 512
#define EEPROM_PAGE_SIZE_BYTES 16


HAL_StatusTypeDef EEPROM_enableWrite(void);
uint8_t EEPROM_buildReadCommand(uint16_t address);
uint8_t EEPROM_buildWriteCommand(uint16_t address);
void EEPROM_csLow(void);
void EEPROM_csHigh(void);
HAL_StatusTypeDef EEPROM_readData(uint16_t address, uint8_t *rxBuffer, uint16_t length);
HAL_StatusTypeDef EEPROM_writeData(uint16_t address, uint8_t *txBuffer, uint16_t length);
HAL_StatusTypeDef EEPROM_writePage(uint16_t address, uint8_t *txBuffer, uint16_t length);



extern SPI_HandleTypeDef hspi1;

#endif
