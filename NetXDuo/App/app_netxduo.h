/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.h
  * @author  MCD Application Team
  * @brief   NetXDuo applicative header file
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
#ifndef __APP_NETXDUO_H__
#define __APP_NETXDUO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "nx_api.h"

/* Private includes ----------------------------------------------------------*/
#include "nx_stm32_eth_driver.h"

/* USER CODE BEGIN Includes */
#include "nxd_dhcp_client.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */
/* The DEFAULT_PAYLOAD_SIZE should match with RxBuffLen configured via MX_ETH_Init */
#ifndef DEFAULT_PAYLOAD_SIZE
#define DEFAULT_PAYLOAD_SIZE      1536
#endif

#ifndef DEFAULT_ARP_CACHE_SIZE
#define DEFAULT_ARP_CACHE_SIZE    1024
#endif

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define PRINT_IP_ADDRESS(addr)         do {                                         \
                                            printf("STM32 %s: %lu.%lu.%lu.%lu \n", #addr, \
                                            (addr >> 24) & 0xff,                    \
                                            (addr >> 16) & 0xff,                    \
                                            (addr >> 8) & 0xff,                     \
                                            (addr & 0xff));                         \
                                       } while(0)

#define PRINT_DATA(addr, port, data)   do {                                           \
                                            printf("[%lu.%lu.%lu.%lu:%u] -> '%s' \n", \
                                            (addr >> 24) & 0xff,                      \
                                            (addr >> 16) & 0xff,                      \
                                            (addr >> 8) & 0xff,                       \
                                            (addr & 0xff), port, data);               \
                                       } while(0)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_NetXDuo_Init(VOID *memory_ptr);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
typedef enum _macAddress{
    MAC_ADDRESS_ID1=62,
    MAC_ADDRESS_ID2=MAC_ADDRESS_ID1+2,
    MAC_ADDRESS_ID3=MAC_ADDRESS_ID2+2,
    MAC_ADDRESS_ID4=MAC_ADDRESS_ID3+2,
    MAC_ADDRESS_ID5=MAC_ADDRESS_ID4+2,
    MAC_ADDRESS_ID6=MAC_ADDRESS_ID5+2,
    MAC_ADDRESS_ID7=MAC_ADDRESS_ID6+2,
    MAC_ADDRESS_ID8=MAC_ADDRESS_ID7+2
}t_macAdress;

#define QUEUE_MAX_SIZE           512

#define LINK_PRIORITY            11



#define DEFAULT_PORT            6001
#define MAC_ADDRESS             MAC_ADDRESS_ID1   // 1.Board=61  2.Board=62-...8.Board=68
#define STATION_ID              (MAC_ADDRESS%10)

#define UDP_SERVER_PORT          DEFAULT_PORT
#define UDP_SERVER_ADDRESS       IP_ADDRESS(192, 168, 2, 158)

#define PRINTF_PORT              6000
#define PRINTF_SERVER_ADDRESS    IP_ADDRESS(192, 168, 2, 158)

#define MAX_PACKET_COUNT         4
#define DEFAULT_MESSAGE          "connection active"

#define NX_APP_CABLE_CONNECTION_CHECK_PERIOD  (6 * NX_IP_PERIODIC_RATE)
/* USER CODE END PD */

#define NX_APP_DEFAULT_TIMEOUT               (10 * NX_IP_PERIODIC_RATE)

#define NX_APP_PACKET_POOL_SIZE              ((DEFAULT_PAYLOAD_SIZE + sizeof(NX_PACKET)) * 10)

#define NX_APP_THREAD_STACK_SIZE             2*1024

#define Nx_IP_INSTANCE_THREAD_SIZE           2*1024

#define NX_APP_THREAD_PRIORITY               10

#ifndef NX_APP_INSTANCE_PRIORITY
#define NX_APP_INSTANCE_PRIORITY             NX_APP_THREAD_PRIORITY
#endif

#define NX_APP_DEFAULT_IP_ADDRESS                   0

#define NX_APP_DEFAULT_NET_MASK                     0

/* USER CODE BEGIN 1 */
extern VOID App_UDP_Thread_SendMESSAGE(const char *,unsigned char,unsigned char);
extern VOID App_UDP_Thread_Send_LOG(void);

#define SENDLOG() App_UDP_Thread_Send_LOG()


/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_NETXDUO_H__ */
