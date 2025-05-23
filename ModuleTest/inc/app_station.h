/*
 * app_station.h
 *
 *  Created on: June 01, 2024
 *      Author: SunnyTeknoloji
 */

#ifndef APP_STATION_H
#define APP_STATION_H



#ifdef  APP_STATION_C
#define INTERFACE
#else 
#define INTERFACE	extern
#endif


/*******************************************************************************
* EXPORTED TYPES
********************************************************************************/


/*******************************************************************************
* EXPORTED DEFS
********************************************************************************/
#define TX_APP_MODULE_STACK_SIZE    1024
#define TX_APP_LEDCHECK_STACK_SIZE  512

#define MODULETEST_PRIORITY         12           // module test thread priority

#define START_STATION               1
#define FINISH_STATION              2

#define STATION_MODE                START_STATION        

#define START_MESSAGE               "start"
#define FINISH_MESSAGE              "finish"

#define REPS_TXMESSAGE              10        

/*
#define QUEUE_STACK_SIZE        32*4
#define TRANSCEIVER_PRIORITY    8
#define PORTAL_PRIORITY         9

#define QUEUE_SIZE              (20<<2)
#define MESSAGE_SIZE            5           // 5*32bit = 20char
*/
typedef enum _testState{
    TEST_STATE_IDLE,
    TEST_STATE_PROCESS,
    TEST_STATE_PASSED,
    TEST_STATE_FAILED,
    TEST_STATE_DONE
}t_testState;

typedef enum _raceState{
    RACE_STATE_IDLE,
    RACE_STATE_READY,
    RACE_STATE_START,
    RACE_STATE_FINISH,
    RACE_STATE_MAX,
}t_raceState;

typedef enum _modulType{
    MODULETYPE_START,
    MODULETYPE_FINISH,
    MODULETYPE_MAX,
}t_modulType;

typedef enum _sensorType{
    SENSOR_PHOTOCELL_1,
    SENSOR_PHOTOCELL_2,
    SENSOR_PHOTOCELL_3,
    SENSOR_PHOTOCELL_4,
    SENSOR_COUNT_MAX,
}t_sensorType;

typedef struct _Sensor {
    uint8_t isPinIDLE;
    uint8_t isCaptured;
    uint8_t isDetected;

    uint8_t isMsgAck;

    uint8_t bounceDetected;

    uint8_t bounce;
    uint8_t sendCount;
    unsigned long timestamp;

}t_Sensor;

/*******************************************************************************
* EXPORTED VARS
********************************************************************************/
INTERFACE TX_THREAD thread_stationapp;
INTERFACE TX_THREAD thread_Led1;
INTERFACE TX_THREAD thread_Led2;

INTERFACE TX_SEMAPHORE semaphore_ledblink;
INTERFACE TX_SEMAPHORE semaphore_buttonpress;
INTERFACE TX_SEMAPHORE semaphore_photocell;

INTERFACE char logmsg[75];

INTERFACE const char * RaceLine_State[RACE_STATE_MAX];
INTERFACE const char * Module_Type[MODULETYPE_MAX];

INTERFACE volatile t_Sensor PhotocellSensor[SENSOR_COUNT_MAX];
INTERFACE t_raceState raceState;
INTERFACE t_modulType moduleType;
INTERFACE const GPIO_TypeDef * Photocell_GPIOs[SENSOR_COUNT_MAX];
INTERFACE const uint16_t Photocell_PINs[SENSOR_COUNT_MAX];

/*******************************************************************************
* EXPORTED FUNCTIONS
********************************************************************************/
INTERFACE VOID Station_thread_entry(ULONG initial_param);
INTERFACE VOID LED1_thread_entry(ULONG initial_param);
INTERFACE VOID LED2_thread_entry(ULONG initial_param);

INTERFACE VOID Station_SensorAck_Update(UINT ackmessage);


#undef	INTERFACE
#endif  /* APP_STATION_H */
