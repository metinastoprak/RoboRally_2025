/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "app_threadx.h"

#include "app_station.h"
#include "app_common.h"
#include "app_buttonled.h"





/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern UART_HandleTypeDef hlpuart1;
extern XSPI_HandleTypeDef hospi1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim17;
extern I2C_HandleTypeDef hi2c2;
extern SAI_HandleTypeDef hsai_BlockA1;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern void Error_Handler(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Photocell_1_Pin GPIO_PIN_2
#define Photocell_1_GPIO_Port GPIOE
#define Photocell_1_EXTI_IRQn EXTI2_IRQn
#define USER_LED2_Pin GPIO_PIN_3
#define USER_LED2_GPIO_Port GPIOE
#define Photocell_3_Pin GPIO_PIN_5
#define Photocell_3_GPIO_Port GPIOF
#define Photocell_3_EXTI_IRQn EXTI5_IRQn
#define LINE2_LED_Pin GPIO_PIN_0
#define LINE2_LED_GPIO_Port GPIOC
#define LINE1_LED_Pin GPIO_PIN_3
#define LINE1_LED_GPIO_Port GPIOC
#define YELLOW_LEDS_Pin GPIO_PIN_0
#define YELLOW_LEDS_GPIO_Port GPIOA
#define IR_RX_Pin GPIO_PIN_5
#define IR_RX_GPIO_Port GPIOA
#define LINE4_LED_Pin GPIO_PIN_0
#define LINE4_LED_GPIO_Port GPIOB
#define LINE3_LED_Pin GPIO_PIN_1
#define LINE3_LED_GPIO_Port GPIOB
#define Photocell_4_Pin GPIO_PIN_15
#define Photocell_4_GPIO_Port GPIOF
#define Photocell_4_EXTI_IRQn EXTI15_IRQn
#define TRIG1_Pin GPIO_PIN_7
#define TRIG1_GPIO_Port GPIOE
#define USER_LED1_Pin GPIO_PIN_15
#define USER_LED1_GPIO_Port GPIOB
#define Photocell_2_Pin GPIO_PIN_6
#define Photocell_2_GPIO_Port GPIOG
#define Photocell_2_EXTI_IRQn EXTI6_IRQn
#define IR_TX_Pin GPIO_PIN_5
#define IR_TX_GPIO_Port GPIOB
#define USER_BUTTON_Pin GPIO_PIN_0
#define USER_BUTTON_GPIO_Port GPIOE
#define USER_BUTTON_EXTI_IRQn EXTI0_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
