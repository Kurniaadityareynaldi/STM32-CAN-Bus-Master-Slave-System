/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "fonts.h"
#include "ssd1306.h"
#include <stdio.h>

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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

CAN_HandleTypeDef hcan;

I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CAN_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//void OLED_DisplayTemperature(void);

CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;

uint8_t TxData[8];
uint8_t RxData[8];

uint32_t TxMailbox;

uint16_t readValue[1];
uint16_t pot;
uint16_t PWM;

float suhu;
#define HIGH 1
#define LOW 0

uint8_t switch_1;
//uint8_t persen;
//uint8_t suhu;
//uint8_t received_suhu;

float received_suhu;
uint8_t received_percentage;
uint8_t received_switch;

int datacheck = 0;

ADC_ChannelConfTypeDef sConfigPrivate = {0};


#define LED_Pin_1 GPIO_PIN_4
#define LED_Port_1 GPIOA
#define LED_Pin_2 GPIO_PIN_5
#define LED_Port_2 GPIOA
//#define LED_Pin_2 GPIO_PIN_6
//#define LED_Port_2 GPIOA

///////TEST_LED_1_NODE/////
//void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
//{
//	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, RxData);
//	if (RxHeader.DLC == 2)
//	{
//		datacheck = 1;
//	}
//}
///////////////////////////


/////////TEST_LED_DIMMING_SENSOR_SUHU_LED_SWITCH_1_NODE////////
//void send_CAN()
//{
//	TxHeader.DLC = 1;
//	TxHeader.IDE = CAN_ID_STD;
//	TxHeader.RTR = CAN_RTR_DATA;
//	TxHeader.StdId = 0x446;  // ID
//
//	TxData[0] = PWM;
//
//	HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
//	HAL_Delay(10);
//}
//
//
//void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
//{
//	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, RxData) == HAL_OK)
//	{
//	   if(RxHeader.StdId == 0x103)
//	   {
//		  RxData[0] = suhu;
//
//		  if(RxData[1] == LOW)
//		  {
//			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
//		  }
//		  else
//		  {
//			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
//		  }
//		  if(RxData[2] == LOW)
//		  {
//			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
//		  }
//		  else
//		  {
//			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
//		  }
//	   }
//	}
//}
//
//void OLED_DisplayTemperature(float suhu)
//{
//    char buffer[16];
//    // Tampilkan suhu
//    sprintf(buffer, "%.1f C", suhu);
//    // Tampilkan "Temperature: " di samping
//    SSD1306_GotoXY(0, 0); // Sesuaikan posisi X dan Y sesuai kebutuhan Anda
//    SSD1306_Puts("Suhu: ", &Font_7x10, SSD1306_COLOR_WHITE);
//    SSD1306_GotoXY(35, 0);
//    SSD1306_Puts("       ", &Font_7x10, SSD1306_COLOR_WHITE);
//    SSD1306_GotoXY(35, 0);
//    SSD1306_Puts(buffer, &Font_7x10, SSD1306_COLOR_WHITE);
//
//    SSD1306_UpdateScreen();
//}
///////////////////////////////////////////////////////////////


/////////////TEST_CAN_2_NODE/////////////////////////////////
typedef enum
{
	LED_BRIGHTNESS,
	SWITCH_1,
} CAN_PACKET_MASTER;

void send_can_data(uint8_t packet_master, const uint8_t* buffer, size_t len)
{
	TxHeader.DLC = len + 1;  // data length
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.StdId = 0x446;  // ID
	TxHeader.ExtId = 0x00;
	TxHeader.TransmitGlobalTime = DISABLE;

	TxData[0] = packet_master;
	memcpy(TxData + 1, buffer, len);

	HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
	HAL_Delay(10);
}

void send_led_brightness(uint8_t PWM)
{
	send_can_data(LED_BRIGHTNESS, (const uint8_t*) &PWM, 1);
}

void send_switch_1(uint8_t switch_1)
{
	send_can_data(SWITCH_1, (const uint8_t*) &switch_1, 1);
}


//void send_CAN()
//{
//	TxHeader.DLC = 2;  // data length
//	TxHeader.IDE = CAN_ID_STD;
//	TxHeader.RTR = CAN_RTR_DATA;
//	TxHeader.StdId = 0x446;  // ID
//	TxHeader.ExtId = 0x00;
//	TxHeader.TransmitGlobalTime = DISABLE;
//
//	TxData[0] = PWM;
//	TxData[1] = switch_1;
//
//	HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
//	HAL_Delay(10);
//}

typedef enum {
	TEMPERATURE,
	PERCENTAGE,
	SWITCH,
	STRING,
} CAN_PACKET_SLAVE_1;

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, RxData) == HAL_OK)
	{
	   if(RxHeader.StdId == 0x211)
	   {
		   switch (RxData[0]) {
		   case TEMPERATURE:
			   memcpy(&received_suhu, &RxData[1], 4);
//			   memcpy(&received_suhu, &RxData[1], sizeof(float));
			   break;

		   case PERCENTAGE:
			   memcpy(&received_percentage, &RxData[1], 1);
//			   memcpy(&received_percentage, &RxData[1], sizeof(uint8_t));
			   break;

		   case SWITCH:
			   memcpy(&received_switch, &RxData[1], 1);
//			   memcpy(&received_switch, &RxData[1], sizeof(uint8_t));

			   if(received_switch == LOW)
			   {
				   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
			   }
			   else
			   {
				   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
			   }
			   break;
		   }

//		  memcpy(&received_suhu, &RxData[0], sizeof(received_suhu));
//		  suhu = received_suhu;

//		  if(RxData[1] == LOW)
//		  {
//			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
//		  }
//		  else
//		  {
//			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
//		  }

	   }
	   else if(RxHeader.StdId == 0x215)
	   {
		   if(RxData[0] == LOW)
		   {
		   	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		   }
		   else
		   {
		   	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		   }
	   }
	}
}

void OLED_DisplayTemperature()
{
    char buffer[16];
//	char buffer_suhu[16];
//    double suhu_double = (double)suhu;

//    sprintf(buffer, "%.1f C", suhu_double);
//    sprintf(buffer_suhu, "%.1f C", suhu_double);
    sprintf(buffer, "%.1f C", received_suhu);
    SSD1306_GotoXY(0, 0); // Sesuaikan posisi X dan Y sesuai kebutuhan Anda
    SSD1306_Puts("Suhu: ", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(35, 0);
    SSD1306_Puts("       ", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(35, 0);
    SSD1306_Puts(buffer, &Font_7x10, SSD1306_COLOR_WHITE);
//    SSD1306_Puts(buffer_suhu, &Font_7x10, SSD1306_COLOR_WHITE);

//    char buffer_persen[16];
//    sprintf(buffer_persen, "%d%%", persen);
    sprintf(buffer, "%d%%", received_percentage);
    SSD1306_GotoXY(0, 20); // Sesuaikan posisi X dan Y sesuai kebutuhan Anda
    SSD1306_Puts("Terisi: ", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(50, 20);
    SSD1306_Puts("       ", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(50, 20);
    SSD1306_Puts(buffer, &Font_7x10, SSD1306_COLOR_WHITE);

    SSD1306_UpdateScreen();
}
/////////////////////////////////////////////////////////////


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
  MX_DMA_Init();
  MX_CAN_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */


///////TEST_LED_1_NODE//////
//  TxHeader.DLC = 2;  // data length
//  TxHeader.IDE = CAN_ID_STD;
//  TxHeader.RTR = CAN_RTR_DATA;
//  TxHeader.StdId = 0x446;  // ID
//
//  TxData[0] = 100;   // ms Delay
//  TxData[1] = 40;    // loop rep
//
//  HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
////////////////////////////


/////////TEST_LED_DIMMING_SENSOR_SUHU_LED_SWITCH_1_NODE////////
//  HAL_CAN_Start(&hcan);
//
//  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO1_MSG_PENDING);
//
//  HAL_ADC_Start_DMA(&hadc1, (uint32_t *) readValue, 1);
//  SSD1306_Init();
///////////////////////////////////////////////////////////////


///////////////TEST_CAN_2_NODE/////////////////////////////////
  HAL_CAN_Start(&hcan);

  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO1_MSG_PENDING);

  HAL_ADC_Start_DMA(&hadc1, (uint32_t *) readValue, 1);
  SSD1306_Init();
///////////////////////////////////////////////////////////////



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


///////TEST_LED_1_NODE//////
//	  if (datacheck)
//	  {
//		// blink the LED
//		for (int i=0; i<RxData[1]; i++)
//		{
//		  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//		  HAL_Delay(RxData[0]);
//		}
//
//		datacheck = 0;
//
//		TxData[0] = 100;   // ms Delay
//		TxData[1] = 40;    // loop rep
//
//		HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
//	  }
////////////////////////////



/////////TEST_LED_DIMMING_SENSOR_SUHU_LED_SWITCH_1_NODE////////
//	  pot = (uint16_t) readValue[0];
//	  PWM = (pot * 100) / 4030;
//
//	  send_CAN();
//	  OLED_DisplayTemperature(suhu);
///////////////////////////////////////////////////////////////


/////////////TEST_CAN_2_NODE/////////////////////////////////
	  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_RESET)
	  	  {
		  	  switch_1 = 0;
	  	  }
	  else
	  	  {
		  	  switch_1 = 1;
	  	  }

	  pot = (uint16_t) readValue[0];
	  PWM = (pot * 100) / 4030;

	  send_can_data(LED_BRIGHTNESS, (const uint8_t*) &PWM, 1);
	  send_can_data(SWITCH_1, (const uint8_t*) &switch_1, 1);

//	  send_CAN();
	  OLED_DisplayTemperature();
/////////////////////////////////////////////////////////////

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 18;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = ENABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

/////////////////TEST_CAN_1_NODE/////////////////////////////////
//  CAN_FilterTypeDef canfilterconfig;
//
//  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
//  canfilterconfig.FilterBank = 2;  // which filter bank to use from the assigned ones
//  canfilterconfig.FilterFIFOAssignment = CAN_FILTER_FIFO1;
//  canfilterconfig.FilterIdHigh = 0x103<<5;
//  canfilterconfig.FilterIdLow = 0;
//  canfilterconfig.FilterMaskIdHigh = 0x103<<5;
//  canfilterconfig.FilterMaskIdLow = 0;
//  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
//  canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
//  canfilterconfig.SlaveStartFilterBank = 10;  // how many filters to assign to the CAN1 (master can)
//
//  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);
/////////////////////////////////////////////////////////////////


/////////////TEST_CAN_2_NODE/////////////////////////////////
   CAN_FilterTypeDef canfilterconfig;

   canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
   canfilterconfig.FilterBank = 6;  // which filter bank to use from the assigned ones
   canfilterconfig.FilterFIFOAssignment = CAN_FILTER_FIFO1;
   canfilterconfig.FilterIdHigh = 0x201<<5;
   canfilterconfig.FilterIdLow = 0;
   canfilterconfig.FilterMaskIdHigh = 0x201<<5;
   canfilterconfig.FilterMaskIdLow = 0;
   canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
   canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
   canfilterconfig.SlaveStartFilterBank = 23;  // how many filters to assign to the CAN1 (master can)

   HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);
/////////////////////////////////////////////////////////////



  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
