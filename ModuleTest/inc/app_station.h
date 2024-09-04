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

typedef enum _msgState{
    MSG_STATE_IDLE,
    MSG_STATE_READY,
    MSG_STATE_PROCESS,
}msgState;

/*******************************************************************************
* EXPORTED VARS
********************************************************************************/
INTERFACE TX_THREAD thread_stationapp;
INTERFACE TX_THREAD thread_Led1;
INTERFACE TX_THREAD thread_Led2;

INTERFACE TX_SEMAPHORE semaphore_ledblink;
INTERFACE TX_SEMAPHORE semaphore_buttonpress;
INTERFACE TX_SEMAPHORE semaphore_photocell;

INTERFACE char logmsg[50];


/*******************************************************************************
* EXPORTED FUNCTIONS
********************************************************************************/
INTERFACE VOID Station_thread_entry(ULONG initial_param);
INTERFACE VOID LED1_thread_entry(ULONG initial_param);
INTERFACE VOID LED2_thread_entry(ULONG initial_param);



#undef	INTERFACE
#endif  /* APP_STATION_H */
