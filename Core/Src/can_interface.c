#include "can_interface.h"
#include "charger.h"

static volatile uint8_t bmsFlags = 0;

volatile int bms_can_current_time = 0;
volatile int bms_can_previous_time = 0;
int bms_can_debounce_ms = 1000;

CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];

HAL_StatusTypeDef CAN_Start(void)
{
	return HAL_CAN_Start(&hcan1);
}

HAL_StatusTypeDef CAN_Activate(void)
{
	return HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

HAL_StatusTypeDef CAN_Send(CANMessage *m)
{
    uint32_t previousTime = HAL_GetTick();

    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0) {
    	// DEBUG_PRINT("waiting\n");
    	if(HAL_GetTick() - previousTime > CAN_TIME_OUT_THRESHOLD_MS){
    		return HAL_TIMEOUT;
    	}
    }

	HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(&hcan1, &m->TxHeader, (uint8_t *)m->data, &m->TxMailbox);

    if (status != HAL_OK)
    {
        DEBUG_PRINT("CAN SEND FAILED: %d\n", status);
        DEBUG_PRINT("TX free level: %lu\n", HAL_CAN_GetTxMailboxesFreeLevel(&hcan1));
        DEBUG_PRINT("CAN error code: 0x%08lX\n", HAL_CAN_GetError(&hcan1));
    }

    return status;
}

void CAN_SettingsInit(CANMessage *canMsgPtr, bool isExtended, uint16_t dlc_length)
{
	CAN_Start();
	CAN_Activate();

	canMsgPtr->TxHeader.IDE = (isExtended) ? CAN_ID_EXT : CAN_ID_STD;
	canMsgPtr->TxHeader.ExtId = (isExtended) ? 0x00000000 : 0x000;
	canMsgPtr->TxHeader.RTR = CAN_RTR_DATA;
	canMsgPtr->TxHeader.DLC = dlc_length;

	CAN_FilterTypeDef filter0 = {0};
	filter0.FilterBank = 0;
	filter0.FilterMode = CAN_FILTERMODE_IDMASK;
	filter0.FilterScale = CAN_FILTERSCALE_32BIT;
	filter0.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filter0.FilterActivation = ENABLE;
	filter0.FilterIdHigh = (BMS_PACK_SUMMARY_TWO_CAN_ID << 5) & 0xFFFF;
	filter0.FilterIdLow = 0;
	filter0.FilterMaskIdHigh = 0xFFFF;
	filter0.FilterMaskIdLow = 0xFFFF;

	CAN_FilterTypeDef filter1 = {0};
	filter1.FilterBank = 1;
	filter1.FilterMode = CAN_FILTERMODE_IDMASK;
	filter1.FilterScale = CAN_FILTERSCALE_32BIT;
	filter1.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filter1.FilterActivation = ENABLE;
	filter1.FilterIdHigh = (BMS_BALANCE_STATUS_CAN_ID << 5) & 0xFFFF;
	filter1.FilterIdLow = 0;
	filter1.FilterMaskIdHigh = 0xFFFF;
	filter1.FilterMaskIdLow = 0xFFFF;

	CAN_FilterTypeDef filter2 = {0};
	filter2.FilterBank = 2;
	filter2.FilterMode = CAN_FILTERMODE_IDMASK;
	filter2.FilterScale = CAN_FILTERSCALE_32BIT;
	filter2.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filter2.FilterActivation = ENABLE;
	filter2.FilterIdHigh = (BMS_PACK_SUMMARY_ONE_CAN_ID << 5) & 0xFFFF;
	filter2.FilterIdLow = 0;
	filter2.FilterMaskIdHigh = 0xFFFF;
	filter2.FilterMaskIdLow = 0xFFFF;

	// ----- Filter 3: ELCON Extended ID 0x18FF50E5 -----
	CAN_FilterTypeDef filter3 = {0};
	filter3.FilterBank = 3;
	filter3.FilterMode = CAN_FILTERMODE_IDMASK;
	filter3.FilterScale = CAN_FILTERSCALE_32BIT;
	filter3.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filter3.FilterActivation = ENABLE;
	filter3.FilterIdHigh = (ELCON_OUTPUT_CAN_ID >> 13) & 0xFFFF;
	filter3.FilterIdLow = ((ELCON_OUTPUT_CAN_ID << 3) & 0xFFFF) | (1 << 2);
	filter3.FilterMaskIdHigh = 0xFFFF;
	filter3.FilterMaskIdLow = 0xFFFF;

	CAN_FilterTypeDef filter4 = {0};
	filter4.FilterBank = 4;
	filter4.FilterMode = CAN_FILTERMODE_IDMASK;
	filter4.FilterScale = CAN_FILTERSCALE_32BIT;
	filter4.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filter4.FilterActivation = ENABLE;
	filter4.FilterIdHigh = (BMS_STATE_OF_CHARGE_CAN_ID << 5) & 0xFFFF;
	filter4.FilterIdLow = 0;
	filter4.FilterMaskIdHigh = 0xFFFF;
	filter4.FilterMaskIdLow = 0xFFFF;

	CAN_FilterTypeDef filter5 = {0};
	filter5.FilterBank = 5;
	filter5.FilterMode = CAN_FILTERMODE_IDMASK;
	filter5.FilterScale = CAN_FILTERSCALE_32BIT;
	filter5.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filter5.FilterActivation = ENABLE;
	filter5.FilterIdHigh = (BMS_FAULT_AND_WARNING_SUMMARY_CAN_ID << 5) & 0xFFFF;
	filter5.FilterIdLow = 0;
	filter5.FilterMaskIdHigh = 0xFFFF;
	filter5.FilterMaskIdLow = 0xFFFF;

	// Accept everything (for testing)
	CAN_FilterTypeDef f = {0};
	f.FilterBank = 0;
	f.FilterMode = CAN_FILTERMODE_IDMASK;
	f.FilterScale = CAN_FILTERSCALE_32BIT;
	f.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	f.FilterActivation = ENABLE;
	f.FilterIdHigh = 0x0000;
	f.FilterIdLow = 0x0000;
	f.FilterMaskIdHigh = 0x0000;
	f.FilterMaskIdLow = 0x0000;

	// Apply filters
	HAL_CAN_ConfigFilter(&hcan1, &filter0);
	HAL_CAN_ConfigFilter(&hcan1, &filter1);
	HAL_CAN_ConfigFilter(&hcan1, &filter2);
	HAL_CAN_ConfigFilter(&hcan1, &filter3);
	HAL_CAN_ConfigFilter(&hcan1, &filter4);
	HAL_CAN_ConfigFilter(&hcan1, &filter5);
	// HAL_CAN_ConfigFilter(&hcan1, &f);

	HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void Set_CAN_Id(CANMessage *ptr, uint32_t id, bool isExtended)
{
	if (isExtended)
	{
		ptr->TxHeader.ExtId = id;
	}
	else
	{
		ptr->TxHeader.StdId = id;
	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
	{
		Error_Handler();
	}

	bms_can_current_time = HAL_GetTick();
	int time_difference = bms_can_current_time - bms_can_previous_time;

	if (RxHeader.IDE == CAN_ID_EXT && RxHeader.ExtId == ELCON_OUTPUT_CAN_ID)
	{
		currentBmsAndElconData.ELCON_outputVoltage_dV = ((RxData[0] << 8) | RxData[1]);
		currentBmsAndElconData.ELCON_outputCurrent_dA = ((RxData[2] << 8) | RxData[3]);
		currentBmsAndElconData.ELCON_fault[4] = RxData[4] & 0x10;
		currentBmsAndElconData.ELCON_fault[3] = RxData[4] & 0x08;
		currentBmsAndElconData.ELCON_fault[2] = RxData[4] & 0x04;
		currentBmsAndElconData.ELCON_fault[1] = RxData[4] & 0x02;
		currentBmsAndElconData.ELCON_fault[0] = RxData[4] & 0x01;
	}

	if (RxHeader.IDE == CAN_ID_STD && time_difference > bms_can_debounce_ms)
	{
		if (RxHeader.StdId == BMS_PACK_SUMMARY_ONE_CAN_ID)
		{
			bmsFlags |= FLAG_PACK_SUMMARY_ONE;

			currentBmsAndElconData.BMS_maxCellVoltage_mV = (int16_t)(((uint16_t)RxData[1] << 8) | RxData[0]);
			currentBmsAndElconData.BMS_minCellVoltage_mV = (int16_t)(((uint16_t)RxData[3] << 8) | RxData[2]);
			currentBmsAndElconData.BMS_maxTemp_C = (int8_t)RxData[4];
			currentBmsAndElconData.BMS_minTemp_C = (int8_t)RxData[5];
			currentBmsAndElconData.BMS_sumPackVoltage_cV = (uint16_t)(((uint16_t)RxData[7] << 8) | RxData[6]);
		}
		else if (RxHeader.StdId == BMS_PACK_SUMMARY_TWO_CAN_ID)
		{
			bmsFlags |= FLAG_PACK_SUMMARY_TWO;

			uint16_t statusBits = ((uint16_t)RxData[1] << 8) | RxData[0];

			currentBmsAndElconData.BMS_fault[0] = (statusBits & (1 << 0)) ? 1 : 0;   // Cell Overvolt Fault
			currentBmsAndElconData.BMS_fault[1] = (statusBits & (1 << 1)) ? 1 : 0;   // Cell Undervolt Fault
			currentBmsAndElconData.BMS_fault[2] = (statusBits & (1 << 2)) ? 1 : 0;   // Cell High Temp Fault
			currentBmsAndElconData.BMS_fault[3] = (statusBits & (1 << 3)) ? 1 : 0;   // Cell Low Temp Fault
			currentBmsAndElconData.BMS_fault[4] = (statusBits & (1 << 4)) ? 1 : 0;   // Redundant Voltage Fault
			currentBmsAndElconData.BMS_fault[5] = (statusBits & (1 << 5)) ? 1 : 0;   // Redundant Temp Fault
			currentBmsAndElconData.BMS_fault[6] = (statusBits & (1 << 6)) ? 1 : 0;   // Invalid Data Fault
			currentBmsAndElconData.BMS_fault[7] = (statusBits & (1 << 7)) ? 1 : 0;   // Open Wire Detection Fault
			
			currentBmsAndElconData.BMS_warning[0] = (statusBits & (1 << 8)) ? 1 : 0;   // Cell Overvolt Warning
			currentBmsAndElconData.BMS_warning[1] = (statusBits & (1 << 9)) ? 1 : 0;   // Cell Undervolt Warning
			currentBmsAndElconData.BMS_warning[2] = (statusBits & (1 << 10)) ? 1 : 0;  // Cell High Temp Warning
			currentBmsAndElconData.BMS_warning[3] = (statusBits & (1 << 11)) ? 1 : 0;  // Cell Low Temp Warning
			currentBmsAndElconData.BMS_warning[4] = (statusBits & (1 << 12)) ? 1 : 0;  // Cell Imbalance Warning
			
			currentBmsAndElconData.BMS_packImbalance_mV = (int16_t)(((uint16_t)RxData[3] << 8) | RxData[2]);
			currentBmsAndElconData.BMS_hvSensePackVoltage_cV = ((uint16_t)RxData[5] << 8) | RxData[4];
			currentBmsAndElconData.BMS_stateOfCharge = ((uint16_t)RxData[7] << 8) | RxData[6];
		}
		else if (RxHeader.StdId == BMS_STATE_OF_CHARGE_CAN_ID)
		{
			bmsFlags |= FLAG_SOC;
			// TODO: Parse SOC message
		}
		else if (RxHeader.StdId == BMS_FAULT_AND_WARNING_SUMMARY_CAN_ID)
		{
			bmsFlags |= FLAG_FAULT_AND_WARNING_SUMMARY;
			uint8_t faultBits = RxData[2];

			currentBmsAndElconData.BMS_fault[0] = (faultBits & (1 << 0)) ? 1 : 0; // Over Volt
			currentBmsAndElconData.BMS_fault[1] = (faultBits & (1 << 1)) ? 1 : 0; // Under Volt
			currentBmsAndElconData.BMS_fault[2] = (faultBits & (1 << 2)) ? 1 : 0; // Open Wire
			currentBmsAndElconData.BMS_fault[3] = (faultBits & (1 << 3)) ? 1 : 0; // PEC
			currentBmsAndElconData.BMS_fault[4] = (faultBits & (1 << 4)) ? 1 : 0; // Over Temp
			currentBmsAndElconData.BMS_fault[5] = (faultBits & (1 << 5)) ? 1 : 0; // Under Temp
			currentBmsAndElconData.BMS_fault[6] = (faultBits & (1 << 6)) ? 1 : 0; // Redundant Volt
			currentBmsAndElconData.BMS_fault[7] = (faultBits & (1 << 7)) ? 1 : 0; // Redundant Temp
		}
	}
	else if (RxHeader.StdId == BMS_BALANCE_STATUS_CAN_ID)
	{
		bmsFlags |= FLAG_BALANCE;
		currentBmsAndElconData.BMS_balanceStatus = (RxData[0] & 0x01) ? 1 : 0;
	}

	if (bmsFlags ==
		(FLAG_PACK_SUMMARY_ONE | FLAG_PACK_SUMMARY_TWO | FLAG_SOC | FLAG_BALANCE | FLAG_FAULT_AND_WARNING_SUMMARY))
	{
		bms_can_previous_time = bms_can_current_time;
		bmsFlags = 0;
	}
}

void CAN_Balance(CANMessage *ptr, bool balancing_enabled)
{
	uint32_t CAN_ID = 0x604;
	Set_CAN_Id(ptr, CAN_ID, false);
	ptr->data[0] = (balancing_enabled) ? 0x1 : 0x0;
	HAL_Delay(10);
	CAN_Send(ptr);
}

void CAN_Charge(CANMessage *ptr, float chargingLimitsVoltsFloat, float chargingLimitsAmpsFloat, bool charge_enable)
{
	// Check for mailbox instead of delay
	uint32_t CAN_ID = 0x1806E5F4;
	Set_CAN_Id(ptr, CAN_ID, true);

	chargingLimitsVoltsFloat *= 10;
	chargingLimitsAmpsFloat *= 10;
	uint16_t chargingLimitsVolts = (uint16_t)chargingLimitsVoltsFloat;
	uint16_t chargingLimitsAmps = (uint16_t)chargingLimitsAmpsFloat;
	DEBUG_PRINT("CHARGING LIMIT VOLTS: %d, CHARGING LIMIT AMPS: %d\n", chargingLimitsVolts, chargingLimitsAmps);

	ptr->data[0] = (chargingLimitsVolts >> 8) & 0xFF;
	ptr->data[1] = chargingLimitsVolts & 0xFF;
	ptr->data[2] = (chargingLimitsAmps >> 8) & 0xFF;
	ptr->data[3] = chargingLimitsAmps & 0xFF;
	ptr->data[4] = (charge_enable) ? 0x00 : 0x01;
	ptr->data[5] = 0x00;
	ptr->data[6] = 0x00;
	ptr->data[7] = 0x00;

	HAL_Delay(3);
	CAN_Send(ptr);
}
