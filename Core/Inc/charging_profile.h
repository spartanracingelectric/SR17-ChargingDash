#ifndef CHARGING_PROFILE_H
#define CHARGING_PROFILE_H

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_STORED_PROFILES 32
#define MAX_DEFAULT_PROFILES 5
#define MAX_AVAILABLE_PROFILES (MAX_DEFAULT_PROFILES + MAX_STORED_PROFILES)

#define PROFILE_SIZE_BYTES 4
#define CRC_SIZE_BYTES 4
#define PROFILE_DATA_SIZE_BYTES (PROFILE_SIZE_BYTES + CRC_SIZE_BYTES)

typedef struct
{
	uint16_t currentCommand_A;
	uint16_t voltageCommand_V;
	bool isDeletable;
} ChargingProfile;

extern ChargingProfile defaultProfiles[MAX_DEFAULT_PROFILES];
extern ChargingProfile storedProfiles[MAX_STORED_PROFILES];
extern ChargingProfile availableProfiles[MAX_AVAILABLE_PROFILES];

extern CRC_HandleTypeDef hcrc;

HAL_StatusTypeDef ChargingProfile_getStoredProfiles(void);
HAL_StatusTypeDef ChargingProfile_storeProfile(uint16_t index, ChargingProfile *profile);
HAL_StatusTypeDef ChargingProfile_storeAllProfiles(void);
HAL_StatusTypeDef ChargingProfile_deleteProfileByIndex(uint16_t index);
HAL_StatusTypeDef ChargingProfile_deleteProfileByValue(uint16_t currentCommand_A, uint16_t voltageCommand_V);
HAL_StatusTypeDef ChargingProfile_deleteAllProfiles(void);
HAL_StatusTypeDef ChargingProfile_addProfile(uint16_t currentCommand_A, uint16_t voltageComand_V);
bool ChargingProfile_isValid(ChargingProfile *profile);
int ChargingProfile_updateAvailableProfiles(void);

#endif
