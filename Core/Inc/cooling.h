#ifndef COOLING_H
#define COOLING_H

#include "main.h"

extern TIM_HandleTypeDef htim3;

void Cooling_commandFanSpeed(int16_t fanSpeed);
float Cooling_readThermistor(uint16_t *adc_thermistor, uint16_t therm_first_resistance);

#endif
