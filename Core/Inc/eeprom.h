#ifndef EEPROM_H
#define EEPROM

#include "main.h"
#include "stm32f1xx_hal_def.h"

#define EEPROM_READ 0x03
#define EEPROM_WRITE 0x02
#define EEPROM_WRDI 0x04
#define EEPROM_WREN 0x06
#define EEPROM_RDSR 0x05
#define EEPROM_WRSR 0x01

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
