#ifndef CHARGER_H
#define CHARGER_H

#include "can_interface.h"
#include "main.h"
#include <stdbool.h>

#define UPPER_MAX_CELL_CV_THRESH_MV 4200
#define LOWER_MAX_CELL_CV_THRESH_MV 4100 
#define MIN_ALLOWED_IMBAL_MV 50 // TODO: CHECK
#define MAINT_AMPS 0.5
#define HYSTERESIS_VOLTS 0.01

#define ELCON_MAX_POWER_W 6600
#define ELCON_MIN_VOLTAGE_V 350
#define ELCON_MAX_VOLTAGE_V 680
#define ELCON_MAX_CURRENT_A 14
#define ELCON_MIN_CURRENT_A 0

typedef enum
{
	CHARGING_MODE_CONSTANT_CURRENT,
	CHARGING_MODE_CURRENT_TAPER,
	CHARGING_MODE_MAINTENANCE,
	CHARGING_MODE_BALANCING,
} chargingMode;

typedef enum
{
	CHARGER_STATE_IDLE,
	CHARGER_STATE_CHARGING,
	CHARGER_STATE_BALANCING,
} chargerState;

// TODO: Separate BMS and Elcon into different structs
typedef struct
{
	int16_t BMS_averageCellVoltage_mV;
	uint16_t BMS_sumPackVoltage_cV;
	uint16_t BMS_hvSensePackVoltage_cV;
	uint16_t BMS_minCellVoltage_mV;
	uint16_t BMS_maxCellVoltage_mV;
	int8_t BMS_averageTemp_C;
	int8_t BMS_minTemp_C;
	int8_t BMS_maxTemp_C;
	int16_t BMS_stateOfCharge; 
	int16_t BMS_packImbalance_mV;
	bool BMS_balanceStatus;
	bool BMS_fault[5];
	bool BMS_warning[5];
	uint16_t ELCON_outputVoltage_dV;
	uint16_t ELCON_outputCurrent_dA;
	bool ELCON_fault[5];
	/*
	Bit 0: 0 -> no hw fail, 1 -> hw fail
	Bit 1: 0 -> no over temp, 1 -> overtemp
	Bit 2: 0 -> input volt right, 1 -> input volt wrong
	Bit 3: 0 -> batt volt detected, 1 -> batt volt not detected
	Bit 4: 0 -> comms good, 1 -> comms timeout
	*/

} bmsAndElconData;

extern volatile bmsAndElconData currentBmsAndElconData;
extern chargingMode currentChargingMode;
extern chargerState currentChargerState;
extern float LIMIT_VOLTS;
extern float LIMIT_AMPS;
extern uint16_t MAX_ALLOWED_PWR;

void Charger_updateChargingMode();
void Charger_handleCharging(CANMessage *charging_msg, CANMessage *balancing_msg);
bool Charger_isChargerSafe();
bool Charger_isHvilSwitchFlipped();
bool Charger_isReadyToChargeSwitchFlipped();
bool Charger_checkFaultStatus();
void Charger_printBmsAndElconData(const volatile bmsAndElconData *d);
void Charger_printPinStates();

#endif
