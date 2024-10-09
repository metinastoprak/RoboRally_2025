/*
 * app_buttonled.c
 *
 *  Created on: June 01, 2024
 *      Author: SunnyTeknoloji
 */

#define	APP_BUTTONLED_C

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
void app_buttonLed(void) {

    ULONG currentValue = 0;
    UINT state;
    char * name;

    if (tx_semaphore_get(&semaphore_ledblink, TX_NO_WAIT) == TX_SUCCESS)
    {
        name = "LED1 Thread";
        tx_semaphore_info_get(&semaphore_ledblink, NULL, &currentValue, NULL, NULL, NULL);
        printf("\r\n[BUTTON-LED] user button pressed\r\n");

        if (tx_thread_info_get(&thread_Led1,&name,&state,NULL,NULL,NULL,NULL,NULL,NULL) == TX_SUCCESS) {

            if (state == TX_SUSPENDED){
                // resume LEDx threads    
                printf("[BUTTON-LED] LEDs blinking\r\n");
                tx_thread_resume(&thread_Led1);
                tx_thread_resume(&thread_Led2);
            
            }
            else{
                // test completed, suspend LEDx threads
                tx_thread_suspend(&thread_Led1);
                tx_thread_suspend(&thread_Led2);
                HAL_GPIO_WritePin(USER_LED1_GPIO_Port,USER_LED1_Pin,GPIO_PIN_RESET);
                HAL_GPIO_WritePin(USER_LED2_GPIO_Port,USER_LED2_Pin,GPIO_PIN_RESET);
                printf("[BUTTON-LED] turnoff LEDs \r\n");    
            }
        }
    }

#if 0
    if (tx_semaphore_get(&semaphore_buttonpress, TX_NO_WAIT) == TX_SUCCESS)
    {

      CHAR message[50] = "id1:s id2:s id3:x id4:s id5:f id6:x id7:G id8:f";
      UINT pattern = 0x0000;
      UCHAR status = 1;                     // resume that all "x"
      
      for (UINT i = 0; i < SENSOR_COUNT_MAX<<1; i++) {
          UINT index = 4 + i * 6;           // check "idX:s/f/x" 

          if (message[index] == 's') {
              pattern |= (1 << (11 - i));   // "s" --> set bit  "11-8
              status = 0;                    
          } else if (message[index] == 'f') {
              pattern |= (1 << (7 - i));    // "f" --> set bit "3-0" (4-3 5-2 6-1 7-0)
              status = 0;  
          } else if (message[index] != 'x') {
            status = 2;pattern = 0x0000;
            printf("[ST%01d-UDP Receive]-> msg error at %d. character '%c'\r\n", 1,index,message[index]);
            break;                // error case captured
          }
      }

      if (status == 1) {
          pattern = 0xFFFF;  // Eğer hepsi 'x' ise sonuç 0xFFFF olacak
      }
      if (pattern != 0) {
          // valid pattern received
          printf("[ST%01d-UDP Receive]-> pattern:0x%04X\r\n", 1,pattern);
          Station_SensorAck_Update(pattern);        //send pattern
      }

    }
#endif    
}

/**
 * @brief  LED1_thread_entry function
 * @param  None
 * @retval None
*/
VOID LED1_thread_entry(ULONG initial_param){
    

    printf("[Thread-LED1] Entry\r\n");    
    while (1)
    {
        tx_thread_sleep(100);        //100ms sleep
        HAL_GPIO_TogglePin(USER_LED1_GPIO_Port,USER_LED1_Pin);
    }
}
/**
 * @brief  LED2_thread_entry function
 * @param  None
 * @retval None
*/
VOID LED2_thread_entry(ULONG initial_param){
    
    printf("[Thread-LED2] Entry\r\n");    
    while (1)
    {
        tx_thread_sleep(200);        //100ms sleep
        HAL_GPIO_TogglePin(USER_LED2_GPIO_Port,USER_LED2_Pin);
    }
}
