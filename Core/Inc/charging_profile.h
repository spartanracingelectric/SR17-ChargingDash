#ifndef CHARGING_PROFILE_H
#define CHARGING_PROFILE_H

#include "main.h"
#include <stdint.h>

#define MAX_STORED_PROFILES 32
#define MAX_DEFAULT_PROFILES 5

#define PROFILE_SIZE_BYTES 4
#define CRC_SIZE_BYTES 4
#define PROFILE_DATA_SIZE_BYTES (PROFILE_SIZE_BYTES + CRC_SIZE_BYTES)

typedef struct
{
	uint16_t maxPower_W;
	uint16_t voltageCommand_V;
} ChargingProfile;

extern ChargingProfile defaultProfiles[MAX_DEFAULT_PROFILES];
extern ChargingProfile storedProfiles[MAX_STORED_PROFILES];

extern CRC_HandleTypeDef hcrc;

HAL_StatusTypeDef ChargingProfile_getStoredProfiles(void);
HAL_StatusTypeDef ChargingProfile_storeProfile(uint16_t index, ChargingProfile *profile);
HAL_StatusTypeDef ChargingProfile_storeAllProfiles(void);
HAL_StatusTypeDef ChargingProfile_deleteProfile(uint16_t index);
HAL_StatusTypeDef ChargingProfile_addProfile(uint16_t maxPower_W, uint16_t voltageComand_V);
HAL_StatusTypeDef ChargingProfile_deleteAllProfiles(void);

#endif
