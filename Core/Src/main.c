/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ring_buffer.h"
#include "sd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SD_SIZE 0xf00000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
SD_HandleTypeDef hsd1;
int32_t timer1, timer2;
size_t blockno = 0;
uint8_t rx_buff[SD_DATA_BLOCK] = { 0 };
uint8_t response = 0xff;

uint8_t music_buf[20480] = { 0 }; //actual samples
RingBuffer music_ringbuf;
Sample_Type_Def music = { 0 };
uint8_t c, lbuf, rbuf;
uint8_t mysd_need_samples = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
	RingBuffer_Init(&music_ringbuf, music_buf, 20480);
	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_SPI1_Init();
	MX_SPI3_Init();
	MX_TIM2_Init();
	MX_TIM10_Init();
	/* USER CODE BEGIN 2 */
	hsd1.CS_Pin = SD_CS_Pin;
	hsd1.CS_Port = SD_CS_GPIO_Port;
	hsd1.DET_Pin = SD_DET_Pin;
	hsd1.DET_Port = SD_DET_GPIO_Port;
	hsd1.WP_Pin = SD_WP_Pin;
	hsd1.WP_Port = SD_WP_GPIO_Port;
	hsd1.hspi = &hspi3;
//	SD_Init(&hsd1);
	uint32_t i = 0;
	do {
		SD_Init(&hsd1);
	} while ((i = SD_Read_Block(&hsd1, rx_buff, blockno)) == 0);

	for (i = 0; i < 512; ++i) {
		if (rx_buff[i] == 'R') {
			if (rx_buff[i + 1] == 'I') {
				if (rx_buff[i + 2] == 'F') {
					if (rx_buff[i + 3] == 'F') {
						music.address = blockno * 512 + i;
						i += 4;
						music.length = rx_buff[i+4];
						music.length = music.length << 8;
						music.length |= rx_buff[i+3];
						music.length = music.length << 8;
						music.length |= rx_buff[i+2];
						music.length = music.length << 8;
						music.length |= rx_buff[i+1];
						i += 12;
						music.bits_per_sample = rx_buff[i];
						i += 6;
						music.channels = rx_buff[i];
						i += 2;
						music.samplerate = rx_buff[i+3];
						music.samplerate = music.samplerate << 8;
						music.samplerate |= rx_buff[i+2];
						music.samplerate = music.samplerate << 8;
						music.samplerate |= rx_buff[i+1];
						music.samplerate = music.samplerate << 8;
						music.samplerate |= rx_buff[i];
						i+=4;
						break;
					}

				}
			}
		}
	}
	if (music.length != 0) {
		//read samples, write to ringbuff,/**/ start playing
		i += 16;
		for (; i < 512; i++) {
			RingBuffer_PutChar(&music_ringbuf, rx_buff[i]);
		}
		blockno += 512;
		uint32_t tmp=(uint32_t)(SystemCoreClock / (music.samplerate))-1;
		htim10.Init.Period = tmp;
		HAL_TIM_Base_Init(&htim10);

		HAL_TIM_Base_Start_IT(&htim10);
		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
	}
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		if (mysd_need_samples) {
			if (!SD_Read_Block(&hsd1, rx_buff, blockno)) {
				while (1)
					;
			}
			blockno += 512;
			for (int i = 0; i < 512; ++i) {
				RingBuffer_PutChar(&music_ringbuf, rx_buff[i]);

			}
			mysd_need_samples = 0;
		}
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
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
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	//TODO
	if (htim->Instance == TIM10) {
		//pobranie sampli jesli zawartosc bufora jest mniejsza niz np 512
		if (music_ringbuf.len < 2048) {
			mysd_need_samples = 1;
		}
		if (music.channels == 2) {
			RingBuffer_GetChar(&music_ringbuf, &c);
			lbuf = c;
			RingBuffer_GetChar(&music_ringbuf, &c);
			rbuf = c;
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, lbuf);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, rbuf);
		} else {
			RingBuffer_GetChar(&music_ringbuf, &c);
			lbuf = c;
			//snprintf(usart_buf,"%d\n",lbuf);

			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, lbuf);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, lbuf);
		}
	}

}
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
