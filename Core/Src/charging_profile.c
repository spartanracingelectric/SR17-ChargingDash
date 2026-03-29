#include "charging_profile.h"
#include "charger.h"
#include "eeprom.h"
#include "stm32f1xx_hal_crc.h"
#include "stm32f1xx_hal_def.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

ChargingProfile defaultProfiles[MAX_DEFAULT_PROFILES] = {
	{.currentCommand_A = 3, .voltageCommand_V = 525},    {.currentCommand_A = 8, .voltageCommand_V = 550},
	{.currentCommand_A = 5, .voltageCommand_V = 570}, {.currentCommand_A = 10, .voltageCommand_V = 580},
	{.currentCommand_A = 3, .voltageCommand_V = 500},
};

ChargingProfile storedProfiles[MAX_STORED_PROFILES] = {0};
ChargingProfile availableProfiles[MAX_DEFAULT_PROFILES + MAX_STORED_PROFILES] = {0};

HAL_StatusTypeDef ChargingProfile_getStoredProfiles(void)
{
	uint16_t baseAddress = 0x000;

	HAL_StatusTypeDef status;

	for (uint16_t i = 0; i < MAX_STORED_PROFILES; i++)
	{
		uint16_t address = baseAddress + (uint16_t)(i * PROFILE_DATA_SIZE_BYTES);
		uint8_t rxBuffer[PROFILE_DATA_SIZE_BYTES] = {0};

		status = EEPROM_readData(address, rxBuffer, PROFILE_DATA_SIZE_BYTES);
		if (status != HAL_OK)
		{
			return status;
		}

		uint32_t storedCRCValue = ((uint32_t)rxBuffer[4] << 0) | ((uint32_t)rxBuffer[5] << 8) |
								  ((uint32_t)rxBuffer[6] << 16) | ((uint32_t)rxBuffer[7] << 24);
		uint32_t expectedCRCValue = HAL_CRC_Calculate(&hcrc, (uint32_t *)rxBuffer, PROFILE_SIZE_BYTES / 4);
		if (storedCRCValue != expectedCRCValue)
		{
			continue;
		}

		uint16_t currentCommand_A = (uint16_t)rxBuffer[0] | ((uint16_t)rxBuffer[1] << 8);
		uint16_t voltageCommand_V = (uint16_t)rxBuffer[2] | ((uint16_t)rxBuffer[3] << 8);

		storedProfiles[i].currentCommand_A = currentCommand_A;
		storedProfiles[i].voltageCommand_V = voltageCommand_V;
	}

	return status;
}

HAL_StatusTypeDef ChargingProfile_storeProfile(uint16_t index, ChargingProfile *profile)
{
	uint16_t baseAddress = 0x000;
	uint16_t address = baseAddress + (uint16_t)(index * PROFILE_DATA_SIZE_BYTES);

	uint8_t txBuffer[PROFILE_DATA_SIZE_BYTES] = {0};

	// For deleting profiles, store 0x0000
	if (profile->currentCommand_A == 0 && profile->voltageCommand_V == 0)
	{
		return EEPROM_writeData(address, txBuffer, PROFILE_DATA_SIZE_BYTES);
	}

	txBuffer[0] = (uint8_t)(profile->currentCommand_A);
	txBuffer[1] = (uint8_t)(profile->currentCommand_A >> 8);

	txBuffer[2] = (uint8_t)(profile->voltageCommand_V);
	txBuffer[3] = (uint8_t)(profile->voltageCommand_V >> 8);

	uint32_t crcValue = HAL_CRC_Calculate(&hcrc, (uint32_t *)txBuffer, PROFILE_SIZE_BYTES / 4);

	txBuffer[4] = (uint8_t)(crcValue);
	txBuffer[5] = (uint8_t)(crcValue >> 8);
	txBuffer[6] = (uint8_t)(crcValue >> 16);
	txBuffer[7] = (uint8_t)(crcValue >> 24);

	return EEPROM_writeData(address, txBuffer, PROFILE_DATA_SIZE_BYTES);
}

HAL_StatusTypeDef ChargingProfile_storeAllProfiles(void)
{
	HAL_StatusTypeDef status = HAL_OK;

	for (uint16_t i = 0; i < MAX_STORED_PROFILES; i++)
	{
		status = ChargingProfile_storeProfile(i, &storedProfiles[i]);
		if (status != HAL_OK)
		{
			return status;
		}
	}

	return status;
}

HAL_StatusTypeDef ChargingProfile_addProfile(uint16_t maxPower_W, uint16_t voltageComand_V)
{
	ChargingProfile_getStoredProfiles();

	// Find empty profile
	for (int i = 0; i < MAX_STORED_PROFILES; i++)
	{
		if (storedProfiles[i].currentCommand_A == 0 && storedProfiles[i].voltageCommand_V == 0)
		{
			storedProfiles[i].currentCommand_A = maxPower_W;
			storedProfiles[i].voltageCommand_V = voltageComand_V;
			return ChargingProfile_storeAllProfiles();
		}
	}

	// No empty profiles were found
	return HAL_ERROR;
}

HAL_StatusTypeDef ChargingProfile_deleteProfile(uint16_t index)
{
	if (index >= MAX_STORED_PROFILES)
	{
		return HAL_ERROR;
	}

	// Delete profile and shift each profile after to the left
	for (uint16_t i = index; i + 1 < MAX_STORED_PROFILES; i++)
	{
		storedProfiles[i] = storedProfiles[i + 1];
	}

	// Last profile is now a duplicate and needs to be deleted
	storedProfiles[MAX_STORED_PROFILES - 1].currentCommand_A = 0;
	storedProfiles[MAX_STORED_PROFILES - 1].voltageCommand_V = 0;

	return ChargingProfile_storeAllProfiles();
}

HAL_StatusTypeDef ChargingProfile_deleteAllProfiles(void)
{
	HAL_StatusTypeDef status;
	for (int i = 0; i < MAX_STORED_PROFILES; i++)
	{
		status = ChargingProfile_deleteProfile(i);
		if (status != HAL_OK)
		{
			return status;
		}
	}
	return status;
}

int ChargingProfile_updateAvailableProfiles(void)
{
	memset(availableProfiles, 0, sizeof(availableProfiles));

	int availableProfileIndex = 0;

	for (int i = 0; i < MAX_DEFAULT_PROFILES; i++)
	{
		if (ChargingProfile_isValid(&defaultProfiles[i]))
		{
			availableProfiles[availableProfileIndex++] = defaultProfiles[i];
		}
	}

	for (int i = 0; i < MAX_STORED_PROFILES; i++)
	{

		if (ChargingProfile_isValid(&storedProfiles[i]))
		{
			availableProfiles[availableProfileIndex++] = storedProfiles[i];
		}
	}

	int numAvailableProfiles = availableProfileIndex;
	return numAvailableProfiles;
}

bool ChargingProfile_isValid(ChargingProfile *profile)
{
	uint32_t power_W = profile->currentCommand_A * profile->voltageCommand_V;

	if (power_W > ELCON_MAX_POWER_W)
	{
		return false;
	}

	if (profile->voltageCommand_V < ELCON_MIN_VOLTAGE_V || profile->voltageCommand_V > ELCON_MAX_VOLTAGE_V)
	{
		return false;
	}

	if (profile->voltageCommand_V <= currentBmsAndElconData.BMS_sumOfCells)
	{
		return false;
	}

	return true;
}
