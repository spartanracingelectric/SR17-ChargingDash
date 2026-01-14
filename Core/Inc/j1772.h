#ifndef J1772_H
#define J1772_H

#include "main.h"
#include <stdbool.h>

#define J1772_AMPS_PER_PERCENT 0.6

extern TIM_HandleTypeDef htim2;

void J1772_init(void);
float J1772_getMaxCurrent(void);
bool J1772_isPlugConnected(void);


#endif
