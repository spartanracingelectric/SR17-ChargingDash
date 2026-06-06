#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H

#include "main.h"
#include <stdbool.h>

#define FLAG_PACK_SUMMARY_ONE (1 << 0)
#define FLAG_PACK_SUMMARY_TWO (1 << 1)
#define FLAG_SOC (1 << 2)
#define FLAG_BALANCE (1 << 3)

#define BMS_BALANCE_STATUS_CAN_ID 0x604
#define BMS_STATE_OF_CHARGE_CAN_ID 0x621
#define BMS_PACK_SUMMARY_ONE_CAN_ID 0x622
#define BMS_PACK_SUMMARY_TWO_CAN_ID 0x623
#define ELCON_OUTPUT_CAN_ID 0x18FF50E5
#define BMS_FAULT_AND_WARNING_SUMMARY_CAN_ID 0x6B2

#define CAN_TIME_OUT_THRESHOLD_MS 10 

typedef struct
{
	CAN_TxHeaderTypeDef TxHeader;
	uint32_t TxMailbox;
	uint8_t data[8];
} CANMessage;

extern CAN_HandleTypeDef hcan1;
extern CAN_RxHeaderTypeDef RxHeader;
extern uint8_t RxData[8];

HAL_StatusTypeDef CAN_Start(void);
HAL_StatusTypeDef CAN_Activate(void);
HAL_StatusTypeDef CAN_Send(CANMessage *canMsgPtr);
void CAN_SettingsInit(CANMessage *canMsgPtr, bool isExtended, uint16_t dlc_length);
void Set_CAN_Id(CANMessage *ptr, uint32_t id, bool isExtended);
void CAN_Balance(CANMessage *ptr, bool balancing_enabled);
void CAN_Charge(CANMessage *ptr, float chargingLimitsVoltsFloat, float chargingLimitsAmpsFloat, bool charge_enable);

#endif
