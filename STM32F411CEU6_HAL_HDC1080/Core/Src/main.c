/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
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

/*ОПИСАНИЕ ПРИМЕРА
 * Мы используем I2C датчик температуры и влажности HDC1080
 * Сделаем вид, что PC13 - это реле, указывающее на аварийную ситуацию
 * Реле будет включаться при превышении климатических условий: Температура 30°C, Влажность 65%
 * И пусть у нас будет счетчик включений реле.
 * Наша задача - вывести это все на график и произвести анализ данных.
 */
#include "HDC1080.h"
#include "ModbusRTU_slave.h"
#include <stdbool.h>

/*=====================Переменные для ModbusRTU Slave===========================*/
uint8_t ModbusRTU_tx_buffer[256] = { 0, }; //Буфер исходящих данных по ModbusRTU
uint8_t ModbusRTU_rx_buffer[256] = { 0, }; //Буфер входящих данных по ModbusRTU
uint16_t ModbusRTU_rx_len = 0; //Счетчик пришедших байт, когда сработает флаг IDLE
extern bool ModbusRTU_Data_recieved; //Флаг принятых данных и готовых к обработке
extern uint16_t ModbusRTU_Slave_MEM[MODBUSRTU_SLAVE_ENUM_QUANTITY]; //Память устройства ModbusRTU_slave
//Прерывание по IDLE смотри в stm32f4xx_it.c
//Функция void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
/*=====================Переменные для ModbusRTU Slave===========================*/

/*=========================Переменные для HDC1080=========================*/
extern HDC1080_struct HDC1080; //Объявляем структуру цифрового датчика температуры и влажности HDC1080
uint16_t Timer_update_data_HDC1080 = 0; //Таймер для запуска опроса датчика HDC1080.
//Данный таймер будем запускать каждые 50 мс. Работать будет через SysTick
//(см. функцию void SysTick_Handler(void) в stm32f4xx_it.c)
/*=========================Переменные для HDC1080=========================*/

/*=================Переменные для логики нашего примера=================*/
bool Alarm_relay = false; //Реле будет включаться при превышении климатических условий: Температура 30°C, Влажность 65%
uint16_t Alarm_counter = 0; //Cчетчик включений реле
bool Alarm_flag_for_counter = false; //Триггер на увеличенние счетчика
/*=================Переменные для логики нашего примера=================*/

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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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
	MX_I2C1_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */
	HDC1080_init(); //Проинициализируем HDC1080
	HAL_UARTEx_ReceiveToIdle_IT(&huart1, ModbusRTU_rx_buffer, sizeof(ModbusRTU_rx_buffer)); //Ждем прерывание по флагу IDLE для UART1
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		/*=================Опрос датчика HDC1080==========================*/
		//Если таймер досчитал - опросим датчик HDC1080
		if (!Timer_update_data_HDC1080) {
			HDC1080_Run();
			Timer_update_data_HDC1080 = 50; //Снова запустим таймер на 50 мс
		}
		ModbusRTU_fast_Embedded_send_data((float)HDC1080.Temperature, MODBUSRTU_SLAVE_CHANNEL_1_H_REG);
		ModbusRTU_fast_Embedded_send_data((float)HDC1080.Humidity, MODBUSRTU_SLAVE_CHANNEL_2_H_REG);
		/*=================Опрос датчика HDC1080==========================*/


		/*======================Логика примера===========================*/
		if ((HDC1080.Temperature > 30.0f) && (HDC1080.Humidity > 65.0f)){
			Alarm_relay = true;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET); //Обратная логика на плате
			if (Alarm_flag_for_counter == false){
				Alarm_counter++; //Увеличим счетчик
				Alarm_flag_for_counter = true; //Сработал триггер на увеличение счетчика до сброса аварии
			}
		}else{
			Alarm_relay = false;
			Alarm_flag_for_counter = false; //Сбросим триггер на увеличение счетчика
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET); //Обратная логика на плате
		}
		ModbusRTU_fast_Embedded_send_data((float)Alarm_relay, MODBUSRTU_SLAVE_CHANNEL_3_H_REG);
		ModbusRTU_fast_Embedded_send_data((float)Alarm_counter, MODBUSRTU_SLAVE_CHANNEL_4_H_REG);
		/*======================Логика примера===========================*/


		/*====================Обработка запросов по ModbusRTU====================*/
		//Если данные пришли
		if (ModbusRTU_Data_recieved) {
			ModbusRTU_slave_Run(ModbusRTU_rx_buffer, ModbusRTU_rx_len, ModbusRTU_tx_buffer); //Обработаем запрос и отдадим ответ
			ModbusRTU_Data_recieved = false; //Ждем следующий запрос
		}
		/*====================Обработка запросов по ModbusRTU====================*/


	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 12;
	RCC_OscInitStruct.PLL.PLLN = 96;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
		Error_Handler();
	}
	/** Enables the Clock Security System
	 */
	HAL_RCC_EnableCSS();
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
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
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

	/*Configure GPIO pin : PC13 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
