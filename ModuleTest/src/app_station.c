/*
 * app_moduletest.c
 *
 *  Created on: June 01, 2024
 *      Author: SunnyTeknoloji
 */

#define	APP_STATION_C

#include "main.h"
#include "app_station.h"
#include "app_netxduo.h"

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

const char * Generic_State[6] =    {"IDLE", \
                                    "INIT", \
                                    "HEAD_OK", \
                                    "HEAD_FAIL", \
                                    "ERROR", \
                                    "DONE"};


volatile t_Sensor PhotocellSensor[SENSOR_COUNT_MAX];
t_raceState raceState;

const GPIO_TypeDef * Photocell_GPIOs[SENSOR_COUNT_MAX]={Photocell_1_GPIO_Port,Photocell_2_GPIO_Port,Photocell_3_GPIO_Port,Photocell_4_GPIO_Port}; 
const uint16_t Photocell_PINs[SENSOR_COUNT_MAX]={Photocell_1_Pin,Photocell_2_Pin,Photocell_3_Pin,Photocell_4_Pin};

const GPIO_TypeDef * LED_GPIOs[SENSOR_COUNT_MAX]={LINE1_LED_GPIO_Port,LINE2_LED_GPIO_Port,LINE3_LED_GPIO_Port,LINE4_LED_GPIO_Port}; 
const uint16_t LED_PINs[SENSOR_COUNT_MAX]={LINE1_LED_Pin,LINE2_LED_Pin,LINE3_LED_Pin,LINE4_LED_Pin};

/*******************************************************************************
* LOCAL VARIABLES
********************************************************************************/
TX_THREAD portal_ptr;


/*******************************************************************************
* LOCAL FUNCTION PROTOTYPES
********************************************************************************/
VOID Portal_thread_entry(ULONG initial_param);
static void Station_SensorHandler(void);
static void Station_SetDefault(void);
static uint8_t Station_IsAnySensorTriggered(void);

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
    raceState = RACE_STATE_IDLE;
    // initialize Sensors
    Station_SetDefault();

    while (1)
    {
        tx_thread_sleep(10);        //100ms sleep
        app_buttonLed();

        switch(raceState)
        {
            case RACE_STATE_IDLE:
            {
                tx_thread_resume(&thread_Led1);    
                // turn off LEDs YELLOW-GREEN-REDs
                HAL_GPIO_WritePin(YELLOW_LEDS_GPIO_Port,YELLOW_LEDS_Pin,GPIO_PIN_RESET);
                for(uint8_t i=0;i<SENSOR_COUNT_MAX;i++)
                    HAL_GPIO_WritePin((GPIO_TypeDef *)LED_GPIOs[i],LED_PINs[i],GPIO_PIN_RESET);
                
#if STATION_MODE == START_STATION    
                if (tx_semaphore_get(&semaphore_buttonpress, TX_NO_WAIT) == TX_SUCCESS) {
                    // button pressed
                    printf("[BUTTON] pressed, RaceState-->READY\r\n"); 
                    Station_SetDefault();

                    // Turn On YELLOW LED bars, check Sensor States
                    HAL_GPIO_WritePin(YELLOW_LEDS_GPIO_Port,YELLOW_LEDS_Pin,GPIO_PIN_SET);

                    snprintf(logmsg, sizeof(logmsg), "[ST%01d-Station] button pressed->READY",STATION_ID);
                    SENDLOG();
                    raceState = RACE_STATE_READY;
                 }
#elif STATION_MODE == FINISH_STATION
                printf("RaceState-->START\r\n");

                snprintf(logmsg, sizeof(logmsg), "[ST%01d-Station] RaceStat->START",STATION_ID);
                SENDLOG();
                raceState = RACE_STATE_START;
#endif

                break;
            }
            case RACE_STATE_READY:
            {	
                Station_SensorHandler();
                if (Station_IsAnySensorTriggered()) {
                    printf("RaceState-->START\r\n"); 
                    raceState = RACE_STATE_START;
                }   
                break;
            }
            case RACE_STATE_START:
            {	
                Station_SensorHandler();     
                break;
            }
            case RACE_STATE_FINISH:
            {	
                break;
            }
        }
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
        if (raceState == RACE_STATE_START && !Station_IsAnySensorTriggered()) {
            printf("[PORTAL] , RaceState-->IDLE\r\n"); 
            raceState = RACE_STATE_IDLE;
            
            // LOG
            snprintf(logmsg, sizeof(logmsg), "[Portal-->ST%01d] new round->IDLE",STATION_ID);
            SENDLOG();
        }

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

    // check Sensors state
    for (uint8_t i=0;i<SENSOR_COUNT_MAX;i++){
        if (PhotocellSensor[i].isDetected && !PhotocellSensor[i].isPinIDLE && (PhotocellSensor[i].sendCount == 0)) {

            printf("\r\n[Station] Photocell-%d detected\r\n",i+1);
            PhotocellSensor[i].isPinIDLE = 1;
            PhotocellSensor[i].bounce = 0;
            PhotocellSensor[i].timestamp = tx_time_get();
            PhotocellSensor[i].sendCount = REPS_TXMESSAGE;

            // Turn On LED of Line
            HAL_GPIO_WritePin((GPIO_TypeDef *)LED_GPIOs[i],LED_PINs[i],GPIO_PIN_SET);

#if STATION_MODE == START_STATION 
            App_UDP_Thread_SendMESSAGE(START_MESSAGE,1,i);
#elif STATION_MODE == FINISH_STATION
            App_UDP_Thread_SendMESSAGE(FINISH_MESSAGE,1,i+4);
#endif
            --PhotocellSensor[i].sendCount;
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

            if (--PhotocellSensor[i].sendCount == 0){
               PhotocellSensor[i].isDetected = 0;PhotocellSensor[i].isPinIDLE = 0;     
            }
        // Send Msg to Portal ----> msgType-sendCount-StationID(photocellSensorIndex)
#if STATION_MODE == START_STATION    
            App_UDP_Thread_SendMESSAGE(START_MESSAGE,REPS_TXMESSAGE-PhotocellSensor[i].sendCount,i);     
#elif STATION_MODE == FINISH_STATION
            App_UDP_Thread_SendMESSAGE(FINISH_MESSAGE,REPS_TXMESSAGE-PhotocellSensor[i].sendCount,i+4);
#endif

        }

    }

}
  /**
  * @brief  Reset default state all stations
  * @param  None
  * @retval None
  */
static void Station_SetDefault(void){

    for (uint8_t i=0;i<SENSOR_COUNT_MAX;i++){
        PhotocellSensor[i].isPinIDLE = 0;
        PhotocellSensor[i].isDetected = 0;
        PhotocellSensor[i].bounce = 0;
        PhotocellSensor[i].sendCount = 0;
    }
}
  /**
  * @brief  Check any sensor EDGE triggered?
  * @param  None
  * @retval None
  */
static uint8_t Station_IsAnySensorTriggered(void){

    return (PhotocellSensor[SENSOR_PHOTOCELL_1].sendCount > 0 ||\
            PhotocellSensor[SENSOR_PHOTOCELL_2].sendCount > 0 ||\
            PhotocellSensor[SENSOR_PHOTOCELL_3].sendCount > 0 ||\
            PhotocellSensor[SENSOR_PHOTOCELL_4].sendCount > 0) ? 1 : 0;
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

