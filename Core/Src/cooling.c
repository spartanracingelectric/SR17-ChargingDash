#include "cooling.h"
#include <stdint.h>

void Cooling_commandFanSpeed(int16_t fanSpeed)
{
	if (fanSpeed >= 0 & fanSpeed <= 100 - 1)
	{
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, fanSpeed);
	}
}
