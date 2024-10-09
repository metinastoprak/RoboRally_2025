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


t_Sensor PhotocellSensor[SENSOR_COUNT_MAX];

const GPIO_TypeDef * Photocell_GPIOs[SENSOR_COUNT_MAX]={Photocell_1_GPIO_Port,Photocell_2_GPIO_Port,Photocell_3_GPIO_Port,Photocell_4_GPIO_Port}; 
const uint16_t Photocell_PINs[SENSOR_COUNT_MAX]={Photocell_1_Pin,Photocell_2_Pin,Photocell_3_Pin,Photocell_4_Pin};

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
    // initialize Sensors
    for (uint8_t i=0;i<SENSOR_COUNT_MAX;i++){
        PhotocellSensor[i].isPinIDLE = 0;
        PhotocellSensor[i].isDetected = 0;
        PhotocellSensor[i].isMsgAck = 0;
        PhotocellSensor[i].bounce = 0;
        PhotocellSensor[i].sendCount = 0;
    }
    
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
  * @brief  "s" or "f" status checking 
  * @param  ack message received from portal
  * 16bit status --> if ackmsg= 0xFFFF then clear IDLE flags to default case for each station
  *              --> 0000ssss 0000ffff checking
  * @retval None
  */
VOID Station_SensorAck_Update(UINT ackmsg){
    
    UCHAR bitALL = 0;
    if (ackmsg == 0xFFFF){
        // all messages x x x x x x x x
        bitALL = 1;
    }
    else {
#if STATION_MODE == START_STATION    
    ackmsg>>= 8;
#elif STATION_MODE == FINISH_STATION
    ackmsg &= 0x00FF;
#endif
    }

    for (UINT i = 0; i < SENSOR_COUNT_MAX; i++) {
        
        if (bitALL ) {
            // default state
            if (PhotocellSensor[i].sendCount == 0) {
                PhotocellSensor[i].isDetected = 0;
                PhotocellSensor[i].isPinIDLE = 0;
            }
        }
        else if (ackmsg & (1 << (3 - i))) 
        {
            PhotocellSensor[i].sendCount = 0;
            PhotocellSensor[i].isDetected = 0;        
        }
    }
} 
/**
* @brief Function Name Station_SensorHandler()
* handle Photocell sensor signal & transmit message to PORTAL side
*/
static void Station_SensorHandler(void) {
#if 0
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
#endif
    // check Sensors state
    for (uint8_t i=0;i<SENSOR_COUNT_MAX;i++){
        if (PhotocellSensor[i].isDetected && !PhotocellSensor[i].isPinIDLE && (PhotocellSensor[i].sendCount == 0)) {

            printf("\r\n[Station] Photocell-%d detected\r\n",i+1);
            PhotocellSensor[i].isPinIDLE = 1;
            PhotocellSensor[i].bounce = 0;
            PhotocellSensor[i].timestamp = tx_time_get();
            PhotocellSensor[i].sendCount = 4;

#if STATION_MODE == START_STATION    
            App_UDP_Thread_SendMESSAGE(START_MESSAGE,1,i);
#elif STATION_MODE == FINISH_STATION
            App_UDP_Thread_SendMESSAGE(FINISH_MESSAGE,1,i+4);
#endif

        }
        else if (PhotocellSensor[i].isPinIDLE){
            // photocell detected, check "bounce" falling edge signal  of PIN level
            if (HAL_GPIO_ReadPin(Photocell_1_GPIO_Port,Photocell_1_Pin) == 1){
                if (++PhotocellSensor[i].bounce >= 3){
                    PhotocellSensor[i].isPinIDLE = 0;PhotocellSensor[i].isDetected = 0;
                }
            }
            else
                PhotocellSensor[i].bounce = 0;
        }

        // resend message 5times  within 1sec time interval
        if ((PhotocellSensor[i].sendCount > 0) && (tx_time_get() - PhotocellSensor[i].timestamp) >= 100) {
            PhotocellSensor[i].timestamp = tx_time_get();
 
        // Send Msg to Portal ----> msgType-sendCount-StationID(photocellSensorIndex)
#if STATION_MODE == START_STATION    
            App_UDP_Thread_SendMESSAGE(START_MESSAGE,6-PhotocellSensor[i].sendCount,i);     
#elif STATION_MODE == FINISH_STATION
            App_UDP_Thread_SendMESSAGE(FINISH_MESSAGE,6-PhotocellSensor[i].sendCount,i+4);
#endif
            if (--PhotocellSensor[i].sendCount == 0){
               PhotocellSensor[i].isDetected = 0;PhotocellSensor[i].isPinIDLE = 0;     
            }
        }

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

