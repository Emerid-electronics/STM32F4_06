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

TIM_HandleTypeDef htim10;

/* USER CODE BEGIN PV */
uint16_t data_ADC[3];
uint8_t inv_SW = 0;
uint8_t alarm_flag = 0;
uint8_t alarm_reset_flag = 0;

float Vsense, Temperature;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM10_Init(void);
/* USER CODE BEGIN PFP */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
float temp_calc(uint16_t ADC_temperature);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_ADC1_Init();
  MX_TIM10_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_DMA(&hadc1,data_ADC,3);
  HAL_TIM_Base_Start_IT(&htim10); //TIM10 start
  /* USER CODE END 2 */
  uint8_t alarm_flag_tmp = 0;
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  alarm_flag_tmp = alarm_flag;

		if (temp_calc(data_ADC[0]) < 30.0) {
			alarm_flag = 0;
		}
		else if (temp_calc(data_ADC[0]) < 29.0){
			alarm_flag = 0;
			alarm_reset_flag = 0;
		}
		else {
			alarm_flag = 1;
			//HAL_GPIO_WritePin(GPIOD,
			//LED_BLUE_Pin | LED_GREEN_Pin | LED_ORANGE_Pin | LED_RED_Pin,
			//		GPIO_PIN_RESET);
		}

		//if ((!alarm_flag_tmp && alarm_flag) && !alarm_reset_flag) {

		//}
		//else HAL_TIM_Base_Stop_IT(&htim10);

		if (!alarm_flag && !alarm_reset_flag){
			if (data_ADC[1] > 3000) { //0.75 * 4095 = 3000+ Vx
				HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin,
						(GPIO_PIN_SET && !inv_SW)
								|| (GPIO_PIN_RESET && inv_SW)); //a~b + ~ab, gdzie a = SET, b = inv_SW
				HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin,
						(GPIO_PIN_RESET && !inv_SW)
								|| (GPIO_PIN_SET && inv_SW)); //a~b + ~ab, gdzie a = RESET, b = inv_SW
			} else if (data_ADC[1] < 1000) { //0.25 * 4095 = 1000
				HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin,
						(GPIO_PIN_RESET && !inv_SW)
								|| (GPIO_PIN_SET && inv_SW));
				HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin,
						(GPIO_PIN_SET && !inv_SW)
								|| (GPIO_PIN_RESET && inv_SW));
			} else {
				HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin,
						GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin,
						GPIO_PIN_RESET);
			}

			if (data_ADC[2] < 1000) { //Vy
				HAL_GPIO_WritePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin,
						(GPIO_PIN_SET && !inv_SW)
								|| (GPIO_PIN_RESET && inv_SW));
				HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin,
						(GPIO_PIN_RESET && !inv_SW)
								|| (GPIO_PIN_SET && inv_SW));
			} else if (data_ADC[2] > 3000) {
				HAL_GPIO_WritePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin,
						(GPIO_PIN_RESET && !inv_SW)
								|| (GPIO_PIN_SET && inv_SW));
				HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin,
						(GPIO_PIN_SET && !inv_SW)
								|| (GPIO_PIN_RESET && inv_SW));
			} else {
				HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin,
						GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin,
						GPIO_PIN_RESET);
			}

		}


    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 49999; //100Mhz / 50k = 2kHz
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 9999; //2kHz / 10k = 0.2 Hz
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LED_GREEN_Pin|LED_ORANGE_Pin|LED_RED_Pin|LED_BLUE_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : BUTTON_USER_Pin */
	GPIO_InitStruct.Pin = BUTTON_USER_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(BUTTON_USER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SW_BUTTON_Pin */
  GPIO_InitStruct.Pin = SW_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_GREEN_Pin LED_ORANGE_Pin LED_RED_Pin LED_BLUE_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_Pin|LED_ORANGE_Pin|LED_RED_Pin|LED_BLUE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	static uint8_t counter_TIM10 = 0;
	if(htim -> Instance == TIM10 && alarm_flag){

		HAL_GPIO_TogglePin(GPIOD, 1 << (12 + counter_TIM10 % 4));
		//counter_TIM10 % 4 - generacja liczb 0 - 3
		//4 + counter_TIM10 % 4 - generacja liczb 12 - 15
		//1 << (4 + counter_TIM10 % 4) przesuniecie bitowe 0000 0000 0000 0001 o 12 - 15 miejsc w lewo - odpowiednio wynik 0x1000 lub 0x2000 lub 0x4000 lub 0x8000
		//LED_GREEN_PIN = 0x1000 | LED_ORANGE_Pin = 0x2000 | LED_RED_Pin = 0x4000 | LED_BLUE_Pin = 0x8000

		counter_TIM10++;

		if (counter_TIM10 == 8){
			counter_TIM10 = 0;
		//	HAL_TIM_Base_Stop_IT(&htim10);
		}
	}

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == BUTTON_USER_Pin)
		alarm_reset_flag = 1;

	if (GPIO_Pin == SW_BUTTON_Pin)
		inv_SW = (inv_SW + 1) % 2;
}

float temp_calc(uint16_t ADC_temperature_measured) {
	//float Vsense;
	//float Temperature;

	const float V25 = 0.76; // [Volts]
	const float Avg_slope = 0.0025; //[Volts/degree]
	const float SupplyVoltage = 3.0; // [Volts]
	const float ADCResolution = 4096.0;

	Vsense = (ADC_temperature_measured / (ADCResolution - 1)) * SupplyVoltage;
	Temperature = 25 + ((Vsense - V25) / Avg_slope);

	return Temperature;

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
