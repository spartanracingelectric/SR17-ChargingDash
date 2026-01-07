#ifndef CHARGING_PROFILE_H
#define CHARGING_PROFILE_H

#include <stdint.h>
#include "main.h"



typedef struct  {
	char name[5];
	uint16_t power_W;
	uint16_t voltage_V;
} ChargingProfile;


void ChargingProfile_testEEPROM(void);


#endif
