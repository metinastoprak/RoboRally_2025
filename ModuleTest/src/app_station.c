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

const char * RaceLine_State[RACE_STATE_MAX] =  {"<case.IDLE> ->", \
                                                "<case.READY> ->", \
                                                "<case.START> ->", \
                                                "<case.FINISH> ->"};

const char * Module_Type[MODULETYPE_MAX] =  {"[ModuleStart]","[ModuleFinish]"};

volatile t_Sensor PhotocellSensor[SENSOR_COUNT_MAX];
t_raceState raceState;
t_modulType moduleType;

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
    
    raceState = RACE_STATE_IDLE;
    
    snprintf(logmsg, sizeof(logmsg), "%s %s thread started..",Module_Type[moduleType],RaceLine_State[raceState]);
    printf("%s\r\n",logmsg);
    SENDLOG();

    while (1)
    {
        tx_thread_sleep(10);        //100ms sleep
        app_buttonLed();

        switch(raceState)
        {
            case RACE_STATE_IDLE:
            {
                // initialize Sensors
                Station_SetDefault();

                tx_thread_resume(&thread_Led1);    
                // turn off LEDs YELLOW-GREEN-REDs
                HAL_GPIO_WritePin(YELLOW_LEDS_GPIO_Port,YELLOW_LEDS_Pin,GPIO_PIN_RESET);
                for(uint8_t i=0;i<SENSOR_COUNT_MAX;i++)
                    HAL_GPIO_WritePin((GPIO_TypeDef *)LED_GPIOs[i],LED_PINs[i],GPIO_PIN_RESET);
                
#if STATION_MODE == START_STATION
                moduleType = MODULETYPE_START;    
                if (tx_semaphore_get(&semaphore_buttonpress, TX_NO_WAIT) == TX_SUCCESS) {
                    // button pressed

                    // Turn On YELLOW LED bars, check Sensor States
                    HAL_GPIO_WritePin(YELLOW_LEDS_GPIO_Port,YELLOW_LEDS_Pin,GPIO_PIN_SET);

                    snprintf(logmsg, sizeof(logmsg), "%s %s Button pressed",Module_Type[moduleType],RaceLine_State[raceState]);
                    printf("%s\r\n",logmsg);
                    SENDLOG();
                    raceState = RACE_STATE_READY;
                    // case transition
                    snprintf(logmsg, sizeof(logmsg), "%s %s mode entry",Module_Type[moduleType],RaceLine_State[raceState]);
                    printf("%s\r\n",logmsg);
                    SENDLOG();                   
                 }
#elif STATION_MODE == FINISH_STATION
                moduleType = MODULETYPE_FINISH;
                raceState = RACE_STATE_START;

                snprintf(logmsg, sizeof(logmsg), "%s %s mode entry",Module_Type[moduleType],RaceLine_State[raceState]);
                printf("%s\r\n",logmsg);
                SENDLOG();
#endif
                break;
            }
            case RACE_STATE_READY:
            {	
                Station_SensorHandler();
                if (Station_IsAnySensorTriggered()) {
                    raceState = RACE_STATE_START;
                    snprintf(logmsg, sizeof(logmsg), "%s %s mode entry",Module_Type[moduleType],RaceLine_State[raceState]);
                    printf("%s\r\n",logmsg);
                    SENDLOG();
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
                Station_SensorHandler();
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
VOID Station_SensorAck_Update(UINT ackmsg) {
    
    UCHAR bitALL = 0;
    UCHAR statSTARTMSG = ackmsg>>8;         // get start station status

    if (ackmsg == 0xFFFF) {
        // all messages x x x x x x x x
        bitALL = 1;
        if (!Station_IsAnySensorTriggered() && (raceState >= RACE_STATE_START)) {
             
#if STATION_MODE == START_STATION      
            snprintf(logmsg, sizeof(logmsg), "%s %s new round->IDLE",Module_Type[moduleType],RaceLine_State[raceState]);
            printf("%s\r\n",logmsg);
            raceState = RACE_STATE_IDLE;
#elif STATION_MODE == FINISH_STATION
            if (raceState == RACE_STATE_FINISH) {
                snprintf(logmsg, sizeof(logmsg), "%s %s new round->IDLE",Module_Type[moduleType],RaceLine_State[raceState]);
                printf("%s\r\n",logmsg);
                raceState = RACE_STATE_IDLE;
            }
#endif
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
                PhotocellSensor[i].isMsgAck = 0;
            }
        }
        else  
        {
            if (ackmsg & (1 << (3 - i))) {
                PhotocellSensor[i].sendCount = 0;
                PhotocellSensor[i].isDetected = 0;
            }
            // set ACK flag in case of "s" received 
            if (statSTARTMSG & (1 << (3 - i))) {
                PhotocellSensor[i].isMsgAck = 1;
            }
            else
                PhotocellSensor[i].isMsgAck = 0;
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

            PhotocellSensor[i].isPinIDLE = 1;
            PhotocellSensor[i].bounce = 0;
            PhotocellSensor[i].timestamp = tx_time_get();
            PhotocellSensor[i].sendCount = REPS_TXMESSAGE;
           
#if STATION_MODE == START_STATION
            // Turn On LED of Line
            HAL_GPIO_WritePin((GPIO_TypeDef *)LED_GPIOs[i],LED_PINs[i],GPIO_PIN_SET);
            App_UDP_Thread_SendMESSAGE(START_MESSAGE,1,i);
            // LOG
            snprintf(logmsg, sizeof(logmsg), "%s %s <photocell #%01d> detected!",Module_Type[moduleType],RaceLine_State[raceState],i+1);
            printf("%s\r\n",logmsg);
            SENDLOG();
#elif STATION_MODE == FINISH_STATION

            if (PhotocellSensor[i].isMsgAck) {
                // Turn On LED of Line
                HAL_GPIO_WritePin((GPIO_TypeDef *)LED_GPIOs[i],LED_PINs[i],GPIO_PIN_SET);
                App_UDP_Thread_SendMESSAGE(FINISH_MESSAGE,1,i+4);    

                // LOG
                snprintf(logmsg, sizeof(logmsg), "%s %s <photocell #%01d> detected!",Module_Type[moduleType],RaceLine_State[raceState],i+5);
                printf("%s\r\n",logmsg);
                SENDLOG();

                raceState = RACE_STATE_FINISH;
            }
            else {
                 // set default
                PhotocellSensor[i].isPinIDLE = 0;PhotocellSensor[i].isDetected = 0;PhotocellSensor[i].sendCount = 0;
                // LOG
                snprintf(logmsg, sizeof(logmsg), "%s %s start NOT Detected at LINE-%01d !!",Module_Type[moduleType],RaceLine_State[raceState],i+1);
                printf("%s\r\n",logmsg);
                SENDLOG();
            }
#endif
            if (PhotocellSensor[i].sendCount > 0)
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
            if (PhotocellSensor[i].isMsgAck)        // "s" message should be confirmed
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

