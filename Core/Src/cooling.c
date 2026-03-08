#include "cooling.h"
#include <math.h>
#include <stdint.h>

void Cooling_commandFanSpeed(int16_t fanSpeed)
{
	if (fanSpeed >= 0 && fanSpeed <= 99)
	{
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, fanSpeed);
	}
}

//TODO: fix
float Cooling_readThermistor(uint16_t *adc_thermistor, uint16_t therm_first_resistance)
{
	if (adc_thermistor == NULL || *adc_thermistor == 0)
	{
		return -273.15;
	}
	float calibration_a = 1.462805229e-3;
	float calibration_b = 2.310582766e-4;
	float calibration_c = 1.189005910e-7;
	float thermistor_voltage = (*adc_thermistor / 4095.0) * 3.3;
	if (thermistor_voltage >= 3.3f)
	{
		thermistor_voltage = 3.299;
	}
	float therm_resistance = (thermistor_voltage * therm_first_resistance) / (3.3 - thermistor_voltage);
	if (therm_resistance <= 0)
	{
		return -273.15;
	}
	float temperature =
		(1 / (calibration_a + calibration_b * log(therm_resistance) + calibration_c * pow(log(therm_resistance), 3))) -
		273.15;
	return temperature;
}
