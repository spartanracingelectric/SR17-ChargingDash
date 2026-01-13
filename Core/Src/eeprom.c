#include "eeprom.h"
#include "charging_profile.h"
#include "main.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_spi.h"
#include <stdint.h>

HAL_StatusTypeDef EEPROM_enableWrite(void)
{
	EEPROM_csLow();
	uint8_t cmd = EEPROM_WREN;
	HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
	EEPROM_csHigh();
	return status;
}

uint8_t EEPROM_buildReadCommand(uint16_t address)
{
	// Bit 9 of the address is part of the read command
	return (uint8_t)(EEPROM_READ | ((address & 0x0100) ? 0x08 : 0x00));
}

uint8_t EEPROM_buildWriteCommand(uint16_t address)
{
	// Bit 9 of the address is part of the write command
	return (uint8_t)(EEPROM_WRITE | ((address & 0x0100) ? 0x08 : 0x00));
}

void EEPROM_csLow(void)
{
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
}

void EEPROM_csHigh(void)
{
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}

HAL_StatusTypeDef EEPROM_readData(uint16_t address, uint8_t *rxBuffer, uint16_t length)
{
	if (address >= EEPROM_SIZE_BYTES)
	{
		return HAL_ERROR;
	}

	uint8_t cmd[2];
	cmd[0] = EEPROM_buildReadCommand(address);
	cmd[1] = (uint8_t)address;

	EEPROM_csLow();
	HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, cmd, 2, HAL_MAX_DELAY);
	if (status == HAL_OK)
	{
		status = HAL_SPI_Receive(&hspi1, rxBuffer, length, HAL_MAX_DELAY);
	}
	EEPROM_csHigh();
	return status;
}

HAL_StatusTypeDef EEPROM_writeData(uint16_t address, uint8_t *txBuffer, uint16_t length)
{
	if (address + length > EEPROM_SIZE_BYTES)
	{
		return HAL_ERROR;
	}

	HAL_StatusTypeDef status = HAL_OK;

	while (length > 0)
	{
		// Can only write one page at a time
		uint16_t page_offset = address & 0x0F;
		uint16_t space_in_page = EEPROM_PAGE_SIZE_BYTES - page_offset;
		uint16_t chunk = (length < space_in_page) ? length : space_in_page;

		status = EEPROM_writePage(address, txBuffer, chunk);
		if (status != HAL_OK)
		{
			return status;
		}

		address += chunk;
		txBuffer += chunk;
		length -= chunk;
	}

	return status;
}

HAL_StatusTypeDef EEPROM_writePage(uint16_t address, uint8_t *txBuffer, uint16_t length)
{
	if (length > EEPROM_PAGE_SIZE_BYTES || length <= 0)
	{
		// TODO: Add error handling
		return HAL_ERROR;
	}

	if (address >= EEPROM_SIZE_BYTES)
	{
		// TODO: Add error handling
		return HAL_ERROR;
	}

	HAL_StatusTypeDef status = EEPROM_enableWrite();
	if (status != HAL_OK)
	{
		return status;
	}

	uint8_t cmd[2];
	cmd[0] = EEPROM_buildWriteCommand(address);
	cmd[1] = (uint8_t)address;

	EEPROM_csLow();
	status = HAL_SPI_Transmit(&hspi1, cmd, 2, HAL_MAX_DELAY);
	if (status == HAL_OK)
	{
		status = HAL_SPI_Transmit(&hspi1, txBuffer, length, HAL_MAX_DELAY);
	}
	EEPROM_csHigh();

	// TODO: Check when write is finished
	HAL_Delay(10);

	return status;
}
