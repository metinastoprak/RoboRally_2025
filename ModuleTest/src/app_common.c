/*
 * app_common.c
 *
 *  Created on: June 01, 2024
 *      Author: SunnyTeknoloji
 */

#define	APP_COMMON_C

#include "main.h"




/*******************************************************************************
* LOCAL DEFINES
********************************************************************************/


/*******************************************************************************
* LOCAL TYPEDEFS 
********************************************************************************/


/*******************************************************************************
* LOCAL VARIABLES
********************************************************************************/


/*******************************************************************************
* LOCAL FUNCTION PROTOTYPES
********************************************************************************/



/*******************************************************************************
* FUNCTIONS
********************************************************************************/

/**
  * @brief EXTI line detection callbacks
  *  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
  ULONG currentValue = 0;

  if (GPIO_Pin == USER_BUTTON_Pin)
  {
    /* Add additional checks to avoid multiple semaphore puts by successively
    clicking on the user button */
    tx_semaphore_info_get(&semaphore_ledblink, NULL, &currentValue, NULL, NULL, NULL);
    //if (currentValue < 2)
    {
        /* Put the semaphore to release the MainThread */
         tx_semaphore_put(&semaphore_ledblink);
    }
    tx_semaphore_put(&semaphore_buttonpress);
  }

}
/**
  * @brief EXTI line detection callbacks
  *  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
  UCHAR bPinState = 0;
  UINT counter = 0;

  for (UCHAR i=0;i<SENSOR_COUNT_MAX;i++){

    if (Photocell_PINs[i] == GPIO_Pin && !PhotocellSensor[i].isPinIDLE) {
        // sensor num found, set flag & vars
      do {
        if (bPinState == HAL_GPIO_ReadPin(Photocell_GPIOs[i],Photocell_PINs[i]))
        {
          ++counter;
        }
        else
        {
          bPinState = HAL_GPIO_ReadPin(Photocell_GPIOs[i],Photocell_PINs[i]);
          counter = 0;
        }
      }while(counter < 100);
      if (!bPinState){
        PhotocellSensor[i].isDetected = 1;
        //tx_semaphore_put(&semaphore_photocell);
      }
      else
        PhotocellSensor[i].isDetected = 0;
    }
  }
}


/**
  * @brief  Application Delay function.
  *   Delay : number of ticks(thread) to wait (1tick=10ms)
  * @retval None
  */
void App_Delay(ULONG Delay)
{
  ULONG initial_time = tx_time_get();
  while ((tx_time_get() - initial_time) < Delay);
}



