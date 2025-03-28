/* USER CODE BEGIN Header */
// Written by Ayman Alamayri in Dec 2024
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

#include "display.h"
#include "kanoa.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
int selectedButton = 0;
bool selectPressed = false;
bool backPressed = false;
uint32_t currentTime;
uint32_t previousTime;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

// PRINTF SUPPORT VIA UART - BEGIN
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
// PRINTF SUPPORT VIA UART - END

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// CAN test begin

struct CANMessage {
	CAN_TxHeaderTypeDef TxHeader;
	uint32_t TxMailbox;
	uint8_t data[8];
};

HAL_StatusTypeDef CAN_Start() {
	return HAL_CAN_Start(&hcan1);
}

HAL_StatusTypeDef CAN_Activate() {
	return HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void CAN_SettingsInit(struct CANMessage *ptr) {
	CAN_Start();
	CAN_Activate();
	ptr->TxHeader.IDE = CAN_ID_EXT;
	ptr->TxHeader.ExtId = 0x00000000;
	ptr->TxHeader.RTR = CAN_RTR_DATA;
	ptr->TxHeader.DLC = 8;
}

void Set_CAN_Id(struct CANMessage *ptr, uint32_t id) {
	ptr->TxHeader.ExtId = id;
}

HAL_StatusTypeDef CAN_Send(struct CANMessage *ptr) {
	return HAL_CAN_AddTxMessage(&hcan1, &ptr->TxHeader, (uint8_t*) ptr->data, &ptr->TxMailbox);
}

int charging_limit_volts = 395;
int charging_limit_amps = 20;

void CAN_Charge(struct CANMessage *ptr, bool charge_enable) {
    uint32_t CAN_ID = 0x1806E5F4;
    Set_CAN_Id(ptr, CAN_ID);

    // convert dec to hex

//    {
//    	int convertedVolts = charging_limit_volts * 10;
//    	int convertedAmps = charging_limit_amps * 10;
//    	while (voltsQuotient != 0) {
//    		voltsRemainder = convertedVolts % 16;
//    		if (voltsRemainder > 9) {
//
//    		}
//    		convertedVolts = convertedVolts / 16;
//    	}
//    }

    ptr->data[0] = 0x0F;
    ptr->data[1] = 0x6E;
    ptr->data[2] = 0x00;
    ptr->data[3] = 0xC8;
    ptr->data[4] = (charge_enable) ? 0x00 : 0x01;
    ptr->data[5] = 0x00;
    ptr->data[6] = 0x00;
    ptr->data[7] = 0x00;
    HAL_Delay(10);
    CAN_Send(ptr);
}

// CAN test end

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	currentTime = HAL_GetTick();
	//debounce set at 200ms
	if (currentTime - previousTime > 200) {
		//rightmost button is GPIO_PIN_4, leftmost button is GPIO_PIN_11
		if (GPIO_Pin == BTN_BTM_Pin && !selectPressed) {
			selectedButton++;
		}
		else if (GPIO_Pin == GPIO_PIN_12) {
			selectPressed = true;
		}
		else if (GPIO_Pin == GPIO_PIN_11 && !selectPressed) {
			backPressed = true;
		}
		else if (GPIO_Pin == BTN_TOP_Pin && !selectPressed) {
			selectedButton--;
		}
		previousTime = currentTime;
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN1_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  ssd1306_Init();

  // CAN TEST BEGIN
  struct CANMessage msg;
  CAN_SettingsInit(&msg);
  // CAN TEST END

  HAL_Delay(1000); // Wait for inits to finish

  // LOL BEGIN
  {
	  ssd1306_Fill(Black);
	  ssd1306_UpdateScreen();
	  ssd1306_SetCursor(0, 0);
	  ssd1306_DrawBitmap(0, 0, kanoaBootImage, 128, 64, White);
	  ssd1306_SetCursor(70, 15);
	  ssd1306_WriteString("KANOA OS", Font_6x8, White);
	  ssd1306_SetCursor(70, 25);
	  ssd1306_WriteString("v0.1.0", Font_6x8, White);
	  ssd1306_SetCursor(70, 35);
	  ssd1306_WriteString("charging", Font_6x8, White);
	  ssd1306_SetCursor(70, 45);
	  ssd1306_WriteString("solutions", Font_6x8, White);
	  ssd1306_UpdateScreen();
	  HAL_Delay(2000);
  }
  // LOL END

  GPIO_PinState HVIL_SW_STATE;
  GPIO_PinState RTC_SW_STATE;

  char chargingString[30];
  sprintf(chargingString, "%d volts @ %d amps", charging_limit_volts, charging_limit_amps);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  HVIL_SW_STATE = HAL_GPIO_ReadPin(HVIL_SW_GPIO_Port, HVIL_SW_Pin);
	  RTC_SW_STATE = HAL_GPIO_ReadPin(RTC_SW_GPIO_Port, RTC_SW_Pin);

	  ssd1306_Fill(Black);
	  ssd1306_UpdateScreen();

	  if(HVIL_SW_STATE && !RTC_SW_STATE) {
		  CAN_Charge(&msg, true);
		  ssd1306_SetCursor(5, 5);
		  ssd1306_WriteString("Charging:", Font_6x8, White);
		  ssd1306_SetCursor(10, 5);
		  ssd1306_WriteString(chargingString, Font_6x8, White);
		  ssd1306_UpdateScreen();
	  } else {
		  CAN_Charge(&msg, false);
		  ssd1306_SetCursor(5, 5);
		  ssd1306_WriteString("Not charging", Font_6x8, White);
		  ssd1306_UpdateScreen();
	  }

	  HAL_Delay(1000);

//	  uint8_t message[] = {"0x3E"};
//	  /*
//	   * First 3 bits are faults (bms, imd, etc)
//	   * Next 3 bits are soc
//	   * Last 2 are reserved
//	  */
//	  uint8_t deviceAddress = 0x3A << 1;  // Atiny address
//	  HAL_StatusTypeDef status = I2C_SendMessage(deviceAddress, message, 5);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	  if (HAL_GPIO_ReadPin(GPIOC, BTN_SEL_Pin) == GPIO_PIN_RESET)
//	  {
//		 HAL_Delay(500);
//		 HAL_GPIO_TogglePin(GPIOA, LED_BAL_Pin);
//		 while (HAL_GPIO_ReadPin(GPIOC, BTN_SEL_Pin) == GPIO_PIN_RESET);
//	  }
	  //HAL_GPIO_TogglePin(GPIOA, LED_BAL_Pin);

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV5;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_PLL2;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL2_ON;
  RCC_OscInitStruct.PLL2.PLL2MUL = RCC_PLL2_MUL8;
  RCC_OscInitStruct.PLL2.HSEPrediv2Value = RCC_HSE_PREDIV2_DIV5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the Systick interrupt time
  */
  __HAL_RCC_PLLI2S_ENABLE();
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 9;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_3TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_HV_Pin|LED_BAL_Pin|LED_CP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_TC_FLT_GPIO_Port, LED_TC_FLT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_HV_Pin LED_BAL_Pin LED_CP_Pin */
  GPIO_InitStruct.Pin = LED_HV_Pin|LED_BAL_Pin|LED_CP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : HVIL_SW_Pin */
  GPIO_InitStruct.Pin = HVIL_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(HVIL_SW_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_TC_FLT_Pin */
  GPIO_InitStruct.Pin = LED_TC_FLT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_TC_FLT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_TOP_Pin BTN_SEL_Pin BTN_BACK_Pin */
  GPIO_InitStruct.Pin = BTN_TOP_Pin|BTN_SEL_Pin|BTN_BACK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN_BTM_Pin */
  GPIO_InitStruct.Pin = BTN_BTM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN_BTM_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RTC_SW_Pin */
  GPIO_InitStruct.Pin = RTC_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(RTC_SW_GPIO_Port, &GPIO_InitStruct);

    /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
