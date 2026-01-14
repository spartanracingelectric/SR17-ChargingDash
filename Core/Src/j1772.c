#include "j1772.h"
#include "main.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"
#include <stdbool.h>

float dutyCycle;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	uint32_t periodTicks = 0;
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
	{
		periodTicks = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
	}

	if (periodTicks == 0)
	{
		return;
	}

	uint32_t highTicks = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
	dutyCycle = ((float)highTicks / periodTicks) * 100;
}

void J1772_init(void)
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_2);
}

float J1772_getMaxCurrent(void)
{
	return dutyCycle * J1772_AMPS_PER_PERCENT;
}

bool J1772_isPlugConnected(void)
{
	if (HAL_GPIO_ReadPin(GPIOC, PP_SIGNAL_Pin) == GPIO_PIN_SET)
	{
		return true;
	}
	else
	{
		return false;
	}
}
