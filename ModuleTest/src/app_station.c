/*
 * app_moduletest.c
 *
 *  Created on: June 01, 2024
 *      Author: SunnyTeknoloji
 */

#define	APP_STATION_C

#include "main.h"
#include "app_station.h"


//#include "app_netxduo.h"


/*******************************************************************************
* LOCAL DEFINES
********************************************************************************/
#if defined(__ICCARM__)
size_t __write(int file, unsigned char const *ptr, size_t len);
/* New definition from EWARM V9, compatible with EWARM8 */
int iar_fputc(int ch);
#define PUTCHAR_PROTOTYPE int iar_fputc(int ch)
#elif defined ( __CC_ARM ) || defined(__ARMCC_VERSION)
/* ARM Compiler 5/6*/
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#elif defined(__GNUC__)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#endif /* __ICCARM__ */


/*******************************************************************************
* LOCAL TYPEDEFS 
********************************************************************************/

typedef struct _msgQueue {
    uint8_t id;
    uint8_t cmd;
    msgState state;
}t_msgQueue;

const char * Generic_State[6] =    {"IDLE", \
                                    "INIT", \
                                    "HEAD_OK", \
                                    "HEAD_FAIL", \
                                    "ERROR", \
                                    "DONE"};



/*******************************************************************************
* LOCAL VARIABLES
********************************************************************************/
TX_THREAD portal_ptr;


/*******************************************************************************
* LOCAL FUNCTION PROTOTYPES
********************************************************************************/
VOID Portal_thread_entry(ULONG initial_param);
static void Station_SensorHandler(void);

/*******************************************************************************
* FUNCTIONS
********************************************************************************/

  /**
  * @brief  Station thread entry function
  * @param  None
  * @retval None
  */
VOID Station_thread_entry(ULONG initial_param){
    

    printf("[Thread-Station] Entry\n\r");
    isPhotocellDetect = 0;isPhotocellIDLE = 0;

    while (1)
    {
        tx_thread_sleep(10);        //100ms sleep

        app_buttonLed();
        Station_SensorHandler();
        
/*
        switch(msgTransceiver.state)
        {
            case MSG_STATE_IDLE:
            {
                break;
            }
            case MSG_STATE_READY:
            {	
                break;
            }
            case MSG_STATE_PROCESS:
            {	
                break;
            }
        }
*/

    }

    
}
/**
* @brief Function Name Station_SensorHandler()
* handle Photocell sensor signal & transmit message to PORTAL side
*/
static void Station_SensorHandler(void) {

    static unsigned char bounceCNT = 0;
    static ULONG start_time;
    static unsigned char tick = 0;


    if (!isPhotocellIDLE && isPhotocellDetect)
    {
        printf("\r\n[Station] Photocell detected\r\n");
        isPhotocellIDLE = 1;
        bounceCNT = 0;
        start_time = tx_time_get();
        tick = 4;

#if STATION_MODE == START_STATION    
        App_UDP_Thread_SendMESSAGE(START_MESSAGE,1);
#elif STATION_MODE == FINISH_STATION
        App_UDP_Thread_SendMESSAGE(FINISH_MESSAGE,1);
#endif

    }
    else
    {   // photocell detected, check "bounce" falling edge signal  of PIN level

        if (HAL_GPIO_ReadPin(Photocell_GPIO_Port,Photocell_Pin) == 0){
            if (++bounceCNT >= 3){
                isPhotocellIDLE = 0;isPhotocellDetect = 0;
            }
            App_Delay(1);
        }
        else
            bounceCNT = 0;
    }

    // resend message 5times  within 1sec time interval
    if ((tick > 0) && (tx_time_get() - start_time) >= 100) {
        start_time = tx_time_get();
 
#if STATION_MODE == START_STATION    
        App_UDP_Thread_SendMESSAGE(START_MESSAGE,6-tick);
#elif STATION_MODE == FINISH_STATION
        App_UDP_Thread_SendMESSAGE(FINISH_MESSAGE,6-tick);
#endif
        --tick;
    }
}

/**
  * @brief  Retargets the C library printf function to the USART.
  *   None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of putchar here */
  /* e.g. write a character to the USART3 and Loop until the end of transmission */
  HAL_UART_Transmit(&hlpuart1, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}

