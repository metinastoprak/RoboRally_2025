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
    static char bounceButton = 0;

    if (raceState != RACE_STATE_IDLE && HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port,USER_BUTTON_Pin)) {
        tx_semaphore_get(&semaphore_buttonpress, TX_NO_WAIT);
        if (++bounceButton >= 50) {
            printf("[BUTTON LONG] pressed, RaceState-->IDLE\r\n");
            raceState = RACE_STATE_IDLE;
            bounceButton = 0;  
            //tx_semaphore_get(&semaphore_buttonpress, TX_NO_WAIT);      
        }
    }
    else {
        bounceButton = 0;
    }


    if (0)  //(tx_semaphore_get(&semaphore_ledblink, TX_NO_WAIT) == TX_SUCCESS)
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

      CHAR message[50] = "id1:s id2:s id3:x id4:x id5:x id6:x id7:x id8:x";
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
          snprintf(logmsg, sizeof(logmsg), "%s %s pattern:0x%04X",Module_Type[moduleType],RaceLine_State[raceState],pattern);
          printf("%s\r\n",logmsg);
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
        switch(raceState)
        {
            case RACE_STATE_IDLE:
            {
                tx_thread_sleep(100);        
                HAL_GPIO_TogglePin(USER_LED1_GPIO_Port,USER_LED1_Pin);
                break;
            }
            case RACE_STATE_READY:
            {
                tx_thread_sleep(50);        
                HAL_GPIO_TogglePin(USER_LED1_GPIO_Port,USER_LED1_Pin);
                break;
            }
            case RACE_STATE_START:
            {
                tx_thread_sleep(20);        
                HAL_GPIO_TogglePin(USER_LED1_GPIO_Port,USER_LED1_Pin);
                break;
            }
            default:
            {
                tx_thread_sleep(100);break;
            }

        }
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
