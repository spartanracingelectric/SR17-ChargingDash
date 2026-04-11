#include "charger.h"
#include "display.h"

float LIMIT_VOLTS = 0;
float LIMIT_AMPS = 0;
uint16_t MAX_ALLOWED_PWR = 10000;

volatile bmsAndElconData currentBmsAndElconData = {0};
chargingMode currentChargingMode = CHARGING_MODE_CONSTANT_CURRENT;
chargerState currentChargerState = CHARGER_STATE_IDLE;

void Charger_updateChargingMode()
{
	if (currentChargerState == CHARGER_STATE_BALANCING)
	{
		currentChargingMode = CHARGING_MODE_BALANCING;
		return;
	}

	if (currentBmsAndElconData.BMS_maxCellVoltage_mV >= UPPER_MAX_CELL_CV_THRESH_MV)
	{
		if (currentBmsAndElconData.BMS_packImbalance_mV >= MIN_ALLOWED_IMBAL_MV)
		{
			currentChargingMode = CHARGING_MODE_BALANCING;
			return;
		}
		else
		{
			currentChargingMode = CHARGING_MODE_MAINTENANCE;
			return;
		}
	}

	if (currentBmsAndElconData.BMS_maxCellVoltage_mV >= LOWER_MAX_CELL_CV_THRESH_MV)
	{
		currentChargingMode = CHARGING_MODE_CURRENT_TAPER;
		return;
	}

	if (currentBmsAndElconData.BMS_maxCellVoltage_mV < LOWER_MAX_CELL_CV_THRESH_MV)
	{
		currentChargingMode = CHARGING_MODE_CONSTANT_CURRENT;
		return;
	}
}

bool Charger_checkChargerConditions()
{
	if (currentChargerState == CHARGER_STATE_BALANCING)
	{
		return (Charger_isChargerSafe() && Charger_isReadyToChargeSwitchFlipped());
	}
	else if (currentChargerState == CHARGER_STATE_CHARGING)
	{
		return (Charger_isChargerSafe() && Charger_isHvilSwitchFlipped() && Charger_isReadyToChargeSwitchFlipped());
	}
	return false;
}

void Charger_handleCharging(CANMessage *charging_msg, CANMessage *balancing_msg)
{
	// If balancing or charging, always check charging conditions and fault status
	if ((currentChargerState == CHARGER_STATE_CHARGING || currentChargerState == CHARGER_STATE_BALANCING) &&
		(!Charger_checkChargerConditions() || Charger_checkFaultStatus()))
	{
		currentChargerState = CHARGER_STATE_IDLE;
		CAN_Charge(charging_msg, LIMIT_VOLTS, LIMIT_AMPS, false);
		CAN_Balance(balancing_msg, false);
		HAL_GPIO_WritePin(GPIOA, LED_BAL_Pin, GPIO_PIN_RESET); 
		nextDisplayState = DISPLAY_STATE_ERRORS;
		return;
	}

	if (currentChargerState == CHARGER_STATE_IDLE)
	{
		// CAN_Charge(charging_msg, LIMIT_VOLTS, LIMIT_AMPS, false);
		CAN_Balance(balancing_msg, false);
		HAL_GPIO_WritePin(GPIOA, LED_BAL_Pin, GPIO_PIN_RESET);
	}
	else if (currentChargerState == CHARGER_STATE_CHARGING)
	{
		Charger_updateChargingMode();
		if (currentChargingMode == CHARGING_MODE_BALANCING)
		{
			CAN_Charge(charging_msg, LIMIT_VOLTS, LIMIT_AMPS, false);
			CAN_Balance(balancing_msg, true);
			HAL_GPIO_WritePin(GPIOA, LED_BAL_Pin, GPIO_PIN_SET);
		}
		else if (currentChargingMode == CHARGING_MODE_MAINTENANCE)
		{
			float voltage = (float)currentBmsAndElconData.BMS_sumPackVoltage_cV / 100.0f;
			CAN_Charge(charging_msg, voltage, MAINT_AMPS, true);
			CAN_Balance(balancing_msg, false);
			HAL_GPIO_WritePin(GPIOA, LED_BAL_Pin, GPIO_PIN_RESET);
		}
		else if (currentChargingMode == CHARGING_MODE_CURRENT_TAPER)
		{
			float current = LIMIT_AMPS + ((MAINT_AMPS - LIMIT_AMPS) / (UPPER_MAX_CELL_CV_THRESH_MV - LOWER_MAX_CELL_CV_THRESH_MV) * (currentBmsAndElconData.BMS_maxCellVoltage_mV - LOWER_MAX_CELL_CV_THRESH_MV));
			CAN_Charge(charging_msg, LIMIT_VOLTS, current, true);
			CAN_Balance(balancing_msg, false);
			HAL_GPIO_WritePin(GPIOA, LED_BAL_Pin, GPIO_PIN_RESET);
		}
		else if (currentChargingMode == CHARGING_MODE_CONSTANT_CURRENT)
		{
			CAN_Charge(charging_msg, LIMIT_VOLTS, LIMIT_AMPS, true);
			CAN_Balance(balancing_msg, false);
			HAL_GPIO_WritePin(GPIOA, LED_BAL_Pin, GPIO_PIN_RESET);
		}
		else
		{
			CAN_Charge(charging_msg, LIMIT_VOLTS, LIMIT_AMPS, false);
			CAN_Balance(balancing_msg, false);
			HAL_GPIO_WritePin(GPIOA, LED_BAL_Pin, GPIO_PIN_RESET);
		}
	}
	else if (currentChargerState == CHARGER_STATE_BALANCING)
	{
		Charger_updateChargingMode();
		if (currentChargingMode == CHARGING_MODE_BALANCING)
		{
			CAN_Charge(charging_msg, LIMIT_VOLTS, LIMIT_AMPS, false);
			CAN_Balance(balancing_msg, true);
			HAL_GPIO_WritePin(GPIOA, LED_BAL_Pin, GPIO_PIN_SET);
		}
		else
		{
			CAN_Charge(charging_msg, LIMIT_VOLTS, LIMIT_AMPS, false);
			CAN_Balance(balancing_msg, false);
			HAL_GPIO_WritePin(GPIOA, LED_BAL_Pin, GPIO_PIN_RESET);
		}
	}
}

void Charger_printPinStates()
{
	GPIO_PinState IN_HVIL_ESTOP_Pin_State = HAL_GPIO_ReadPin(IN_HVIL_ESTOP_GPIO_Port, IN_HVIL_ESTOP_Pin);
	GPIO_PinState IN_HVIL_TERM_Pin_State = HAL_GPIO_ReadPin(IN_HVIL_TERM_GPIO_Port, IN_HVIL_TERM_Pin);
	GPIO_PinState IN_HVIL_ACUM_Pin_State = HAL_GPIO_ReadPin(IN_HVIL_ACUM_GPIO_Port, IN_HVIL_ACUM_Pin);
	GPIO_PinState IN_HVIL_FSW_STATE = HAL_GPIO_ReadPin(IN_HVIL_FSW_GPIO_Port, IN_HVIL_FSW_Pin);
	GPIO_PinState RTC_SW_STATE = HAL_GPIO_ReadPin(IN_RTC_SW_GPIO_Port, IN_RTC_SW_Pin);
	GPIO_PinState PLUG_CONNECTION_STATE = HAL_GPIO_ReadPin(GPIOC, PP_SIGNAL_Pin);
	DEBUG_PRINT("Charger Pin States\n");
	DEBUG_PRINT("IN_HVIL_ESTOP: %s\n", IN_HVIL_ESTOP_Pin_State == GPIO_PIN_SET ? "SET" : "RESET");
	DEBUG_PRINT("IN_HVIL_TERM : %s\n", IN_HVIL_TERM_Pin_State == GPIO_PIN_SET ? "SET" : "RESET");
	DEBUG_PRINT("IN_HVIL_ACUM : %s\n", IN_HVIL_ACUM_Pin_State == GPIO_PIN_SET ? "SET" : "RESET");
	DEBUG_PRINT("IN_HVIL_FSW  : %s\n", IN_HVIL_FSW_STATE == GPIO_PIN_SET ? "SET" : "RESET");
	DEBUG_PRINT("RTC_SW       : %s\n", RTC_SW_STATE == GPIO_PIN_SET ? "SET" : "RESET");
	DEBUG_PRINT("PLUG STATE   : %s\n", PLUG_CONNECTION_STATE == GPIO_PIN_SET ? "SET" : "RESET");
}

// TODO: Check these conditions
bool Charger_isChargerSafe()
{
	GPIO_PinState IN_HVIL_CHAR_Pin_State;
	GPIO_PinState IN_HVIL_ESTOP_Pin_State = HAL_GPIO_ReadPin(IN_HVIL_ESTOP_GPIO_Port, IN_HVIL_ESTOP_Pin); //SET WHEN NOT PRESSED
	GPIO_PinState IN_HVIL_TERM_Pin_State = HAL_GPIO_ReadPin(IN_HVIL_TERM_GPIO_Port, IN_HVIL_TERM_Pin); //RESET WHEN HVIL SWITCHED OFF
	GPIO_PinState IN_HVIL_ACUM_Pin_State = HAL_GPIO_ReadPin(IN_HVIL_ACUM_GPIO_Port, IN_HVIL_ACUM_Pin); // SET when ACUM HVIL DETECTED
	if (IN_HVIL_ESTOP_Pin_State == GPIO_PIN_SET && IN_HVIL_TERM_Pin_State == GPIO_PIN_SET && IN_HVIL_ACUM_Pin_State == GPIO_PIN_SET)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Charger_isHvilSwitchFlipped()
{
	GPIO_PinState IN_HVIL_FSW_STATE = HAL_GPIO_ReadPin(IN_HVIL_FSW_GPIO_Port, IN_HVIL_FSW_Pin);
	if (IN_HVIL_FSW_STATE)
	{
		HAL_GPIO_WritePin(GPIOA, LED_HV_Pin, GPIO_PIN_SET);
		return true;
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, LED_HV_Pin, GPIO_PIN_RESET);
		return false;
	}
}

bool Charger_isReadyToChargeSwitchFlipped()
{
	GPIO_PinState RTC_SW_STATE = HAL_GPIO_ReadPin(IN_RTC_SW_GPIO_Port, IN_RTC_SW_Pin);
	if (RTC_SW_STATE)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool Charger_checkFaultStatus()
{
	static bool wasFaulting = false;
	static uint32_t faultStart_ms = 0;

	bool nowFaulting = false;
	for (int i = 0; i < 5; i++)
	{
		if (currentBmsAndElconData.ELCON_fault[i])
		{
			nowFaulting = true;
			break;
		}
	}
	for (int i = 0; i < 8; i++)
	{
		if (currentBmsAndElconData.BMS_fault[i])
		{
			nowFaulting = true;
			break;
		}
	}

	uint32_t now = HAL_GetTick();
	if (nowFaulting && !wasFaulting)
	{
		faultStart_ms = now; // Timer starts when fault first appears. Timer will implicitly reset if it stops faulting
	}
	wasFaulting = nowFaulting;

	if (nowFaulting)
	{
		if (now - faultStart_ms >= 2000)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void Charger_printBmsAndElconData(const volatile bmsAndElconData *d)
{
	DEBUG_PRINT("BMS_avgVolt       = %d mV\n", d->BMS_averageCellVoltage_mV);
	DEBUG_PRINT("BMS_sumOfCells    = %d cV\n", d->BMS_sumPackVoltage_cV);
	DEBUG_PRINT("BMS_minVolt       = %d mV\n", d->BMS_minCellVoltage_mV);
	DEBUG_PRINT("BMS_maxVolt       = %d mV\n", d->BMS_maxCellVoltage_mV);
	DEBUG_PRINT("BMS_minTemp       = %d C\n", d->BMS_minTemp_C);
	DEBUG_PRINT("BMS_maxTemp       = %d C\n", d->BMS_maxTemp_C);
	DEBUG_PRINT("BMS_stateOfCharge = %d %%\n", d->BMS_stateOfCharge);
	DEBUG_PRINT("BMS_packImbalance = %d mv\n", d->BMS_packImbalance_mV);
	DEBUG_PRINT("ELCON_outVolt     = %d dV\n", d->ELCON_outputVoltage_dV);
	DEBUG_PRINT("ELCON_outCurrent  = %d dV\n", d->ELCON_outputVoltage_dV);

	// Fault bits: 0=hw fail, 1=overtemp, 2=input volt wrong, 3=batt volt not detected, 4=comms timeout
	DEBUG_PRINT("ELCON_faults      = [");
	for (int i = 0; i < 5; ++i)
	{
		DEBUG_PRINT("%s", d->ELCON_fault[i] ? "1" : "0");
		if (i < 4)
			DEBUG_PRINT(", ");
	}
	DEBUG_PRINT("]\n");
}


