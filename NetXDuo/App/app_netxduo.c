/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
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

/* Includes ------------------------------------------------------------------*/
#include "app_netxduo.h"

/* Private includes ----------------------------------------------------------*/
#include "nxd_dhcp_client.h"
/* USER CODE BEGIN Includes */
#include "main.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TX_THREAD      NxAppThread;
NX_PACKET_POOL NxAppPool;
TX_SEMAPHORE   DHCPSemaphore;
NX_IP          NetXDuoEthIpInstance;
NX_DHCP        DHCPClient;
/* USER CODE BEGIN PV */
TX_THREAD AppUDPThread;
TX_THREAD AppLinkThread;

NX_UDP_SOCKET UDPSocket;
ULONG IpAddress;
ULONG NetMask;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static VOID nx_app_thread_entry (ULONG thread_input);
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr);
/* USER CODE BEGIN PFP */
static VOID App_UDP_Thread_Entry(ULONG thread_input);
static VOID App_Link_Thread_Entry(ULONG thread_input);

/* USER CODE END PFP */

/**
  * @brief  Application NetXDuo Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
  UINT ret = NX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

   /* USER CODE BEGIN App_NetXDuo_MEM_POOL */
  (void)byte_pool;
  /* USER CODE END App_NetXDuo_MEM_POOL */
  /* USER CODE BEGIN 0 */
  printf("[NETX-DUO] Nx_UDP_Echo_Client application started..\n");
  /* USER CODE END 0 */

  /* Initialize the NetXDuo system. */
  CHAR *pointer;
  nx_system_initialize();

    /* Allocate the memory for packet_pool.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, NX_APP_PACKET_POOL_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the Packet pool to be used for packet allocation,
   * If extra NX_PACKET are to be used the NX_APP_PACKET_POOL_SIZE should be increased
   */
  ret = nx_packet_pool_create(&NxAppPool, "NetXDuo App Pool", DEFAULT_PAYLOAD_SIZE, pointer, NX_APP_PACKET_POOL_SIZE);

  if (ret != NX_SUCCESS)
  {
    return NX_POOL_ERROR;
  }

    /* Allocate the memory for Ip_Instance */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, Nx_IP_INSTANCE_THREAD_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

   /* Create the main NX_IP instance */
  ret = nx_ip_create(&NetXDuoEthIpInstance, "NetX Ip instance", NX_APP_DEFAULT_IP_ADDRESS, NX_APP_DEFAULT_NET_MASK, &NxAppPool, nx_stm32_eth_driver,
                     pointer, Nx_IP_INSTANCE_THREAD_SIZE, NX_APP_INSTANCE_PRIORITY);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_SUCCESSFUL;
  }

    /* Allocate the memory for ARP */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, DEFAULT_ARP_CACHE_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Enable the ARP protocol and provide the ARP cache size for the IP instance */

  /* USER CODE BEGIN ARP_Protocol_Initialization */

  /* USER CODE END ARP_Protocol_Initialization */

  ret = nx_arp_enable(&NetXDuoEthIpInstance, (VOID *)pointer, DEFAULT_ARP_CACHE_SIZE);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_SUCCESSFUL;
  }

  /* Enable the ICMP */

  /* USER CODE BEGIN ICMP_Protocol_Initialization */

  /* USER CODE END ICMP_Protocol_Initialization */

  ret = nx_icmp_enable(&NetXDuoEthIpInstance);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_SUCCESSFUL;
  }

  /* Enable TCP Protocol */

  /* USER CODE BEGIN TCP_Protocol_Initialization */

  /* USER CODE END TCP_Protocol_Initialization */

  ret = nx_tcp_enable(&NetXDuoEthIpInstance);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_SUCCESSFUL;
  }

  /* Enable the UDP protocol required for  DHCP communication */

  /* USER CODE BEGIN UDP_Protocol_Initialization */

  /* USER CODE END UDP_Protocol_Initialization */

  ret = nx_udp_enable(&NetXDuoEthIpInstance);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_SUCCESSFUL;
  }

   /* Allocate the memory for main thread   */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, NX_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the main thread */
  ret = tx_thread_create(&NxAppThread, "NetXDuo App thread", nx_app_thread_entry , 0, pointer, NX_APP_THREAD_STACK_SIZE,
                         NX_APP_THREAD_PRIORITY, NX_APP_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);

  if (ret != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* Create the DHCP client */

  /* USER CODE BEGIN DHCP_Protocol_Initialization */

  /* USER CODE END DHCP_Protocol_Initialization */

  ret = nx_dhcp_create(&DHCPClient, &NetXDuoEthIpInstance, "DHCP Client");

  if (ret != NX_SUCCESS)
  {
    return NX_DHCP_ERROR;
  }

  /* set DHCP notification callback  */
  tx_semaphore_create(&DHCPSemaphore, "DHCP Semaphore", 0);

  /* USER CODE BEGIN MX_NetXDuo_Init */
    /* Allocate the memory for UDP client thread   */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,NX_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }
  /* create the UDP client thread */
  ret = tx_thread_create(&AppUDPThread, "App UDP Thread", App_UDP_Thread_Entry, 0, pointer, NX_APP_THREAD_STACK_SIZE,
                         NX_APP_THREAD_PRIORITY, NX_APP_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);

  if (ret != TX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }
  /* Allocate the memory for Link thread   */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, NX_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* create the Link thread */
  ret = tx_thread_create(&AppLinkThread, "App Link Thread", App_Link_Thread_Entry, 0, pointer, NX_APP_THREAD_STACK_SIZE,
                         LINK_PRIORITY, LINK_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);

  if (ret != TX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }
  /* USER CODE END MX_NetXDuo_Init */

  return ret;
}

/**
* @brief  ip address change callback.
* @param ip_instance: NX_IP instance
* @param ptr: user data
* @retval none
*/
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr)
{
  /* USER CODE BEGIN ip_address_change_notify_callback */

  /* USER CODE END ip_address_change_notify_callback */

  /* release the semaphore as soon as an IP address is available */
  tx_semaphore_put(&DHCPSemaphore);
}

/**
* @brief  Main thread entry.
* @param thread_input: ULONG user argument used by the thread entry
* @retval none
*/
static VOID nx_app_thread_entry (ULONG thread_input)
{
  /* USER CODE BEGIN Nx_App_Thread_Entry 0 */

  /* USER CODE END Nx_App_Thread_Entry 0 */

  UINT ret = NX_SUCCESS;

  /* USER CODE BEGIN Nx_App_Thread_Entry 1 */

  /* USER CODE END Nx_App_Thread_Entry 1 */

  /* register the IP address change callback */
  ret = nx_ip_address_change_notify(&NetXDuoEthIpInstance, ip_address_change_notify_callback, NULL);
  if (ret != NX_SUCCESS)
  {
    /* USER CODE BEGIN IP address change callback error */
    Error_Handler();
    /* USER CODE END IP address change callback error */
  }

  /* start the DHCP client */
  ret = nx_dhcp_start(&DHCPClient);
  if (ret != NX_SUCCESS)
  {
    /* USER CODE BEGIN DHCP client start error */
    printf("[NETXAPP-THREAD] DHCP client start error\r\n");
    Error_Handler();
    /* USER CODE END DHCP client start error */
  }

  /* wait until an IP address is ready */
  if(tx_semaphore_get(&DHCPSemaphore, NX_APP_DEFAULT_TIMEOUT) != TX_SUCCESS)
  {
    /* USER CODE BEGIN DHCPSemaphore get error */
    printf("[NETXAPP-THREAD] IP get error-->retry\r\n");

    while (1) {
        /* IP adresi almak için bekle */
        if (tx_semaphore_get(&DHCPSemaphore, NX_APP_DEFAULT_TIMEOUT) == TX_SUCCESS)
        {
            /* get IP success */
            break;
        }
        printf("[NETXAPP-THREAD] IP get error, retrying...\r\n");
        tx_thread_sleep(NX_IP_PERIODIC_RATE);

        /* re-start DHCP */
        nx_dhcp_stop(&DHCPClient);
        nx_dhcp_start(&DHCPClient);
    }   
    /* USER CODE END DHCPSemaphore get error */
  }

  /* USER CODE BEGIN Nx_App_Thread_Entry 2 */
  /* get IP address */
  ret = nx_ip_address_get(&NetXDuoEthIpInstance, &IpAddress, &NetMask);

  /* print the IP address */
  //PRINT_IP_ADDRESS(IpAddress);

  snprintf(logmsg, sizeof(logmsg), "%s %s IP: %lu.%lu.%lu.%lu \r\n",Module_Type[moduleType],RaceLine_State[raceState], \
          (IpAddress >> 24) & 0xff, \
          (IpAddress >> 16) & 0xff, \
          (IpAddress >> 8) & 0xff,  \
          (IpAddress & 0xff));
  printf("%s\r\n",logmsg);
  SENDLOG();


  if (ret != TX_SUCCESS)
  {
    Error_Handler();
  }
  /* the network is correctly initialized, start the UDP thread */
  tx_thread_resume(&AppUDPThread);


  /* this thread is not needed any more, we relinquish it */
  tx_thread_relinquish();

  return;
  /* USER CODE END Nx_App_Thread_Entry 2 */

}
/* USER CODE BEGIN 1 */
static VOID App_UDP_Thread_Entry(ULONG thread_input)
{
  UINT ret;
  ULONG bytes_read;
  NX_PACKET *server_packet;
  UCHAR data_buffer[512];

  NX_PACKET *data_packet;
  //CHAR message[30];

  /* create the UDP socket */
  ret = nx_udp_socket_create(&NetXDuoEthIpInstance, &UDPSocket, "UDP Client Socket", NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, QUEUE_MAX_SIZE);

  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* bind UDP socket to the DEFAULT PORT */
  ret = nx_udp_socket_bind(&UDPSocket, DEFAULT_PORT, TX_WAIT_FOREVER);

  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

	printf("\rUDP_Thread_Entry\n");

  while(1)
  {

    TX_MEMSET(data_buffer, '\0', sizeof(data_buffer));

    /* create the packet to send over the UDP socket */
    ret = nx_packet_allocate(&NxAppPool, &data_packet, NX_UDP_PACKET, TX_WAIT_FOREVER);

    if (ret != NX_SUCCESS)
    {
      Error_Handler();
    }

     //send connection active
//    TX_MEMSET(message, '\0', sizeof(message));
//    snprintf(message, sizeof(message), "id:%01d connection active", STATION_ID);
    TX_MEMSET(logmsg, '\0', sizeof(logmsg));
    snprintf(logmsg, sizeof(logmsg), "%s IP: %lu.%lu.%lu.%lu \r\n",Module_Type[moduleType], \
            (IpAddress >> 24) & 0xff, \
            (IpAddress >> 16) & 0xff, \
            (IpAddress >> 8) & 0xff,  \
            (IpAddress & 0xff));
    printf("%s\r\n",logmsg);
    SENDLOG();


    ret = nx_packet_data_append(data_packet, (VOID *)logmsg, strlen(logmsg), &NxAppPool, TX_WAIT_FOREVER);
    //ret = nx_packet_data_append(data_packet, (VOID *)DEFAULT_MESSAGE, sizeof(DEFAULT_MESSAGE), &NxAppPool, TX_WAIT_FOREVER);
    if (ret != NX_SUCCESS)
    {
        Error_Handler();
    }

      /* send the message */
      ret = nx_udp_socket_send(&UDPSocket, data_packet, UDP_SERVER_ADDRESS, UDP_SERVER_PORT);

      /* MSG Listening .. wait 10 sec to receive response from the server */
      ret = nx_udp_socket_receive(&UDPSocket, &server_packet, NX_APP_DEFAULT_TIMEOUT);

    if (ret == NX_SUCCESS)
    {
      ULONG source_ip_address;
      UINT source_port;

      /* get the server IP address and  port */
      nx_udp_source_extract(server_packet, &source_ip_address, &source_port);

      /* retrieve the data sent by the server */
      nx_packet_data_retrieve(server_packet, data_buffer, &bytes_read);

      printf("%s %s receive-> %s\r\n", Module_Type[moduleType],RaceLine_State[raceState],data_buffer);

      // check received message characters
      // message format ex: "id1:s id2:s id3:x id4:x id5:f id6:x id7:x id8:x"
      // if all chars = "x" then msgpattern= 0xFFFF --> station enters default mode
      // pattern: "0b0000-ssss-0000-ffff"

      UINT pattern = 0x0000;
      UCHAR status = 1;                     // resume that all "x"
      for (UINT i = 0; i < SENSOR_COUNT_MAX<<1; i++) {
          UINT index = 4 + i * 6;           // check "idX:s/f/x" 

          if (data_buffer[index] == 's') {
              pattern |= (1 << (11 - i));   // "s" --> set bit  "11-8
              status = 0;                    
          } else if (data_buffer[index] == 'f') {
              pattern |= (1 << (7 - i));    // "f" --> set bit "3-0" 
              status = 0;  
          } else if (data_buffer[index] != 'x') {
            status = 2;pattern = 0x0000;
            // Log
            snprintf(logmsg, sizeof(logmsg), "%s %s msg error at %d. character '%c'",Module_Type[moduleType],RaceLine_State[raceState],index,data_buffer[index]);
            printf("%s\r\n",logmsg);
            SENDLOG();
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
          SENDLOG();
          Station_SensorAck_Update(pattern);        //send pattern
      }

#if 0   
	 // get "id:"  "cmd:" "stat:" sub msg
	 id_pos = strstr(data_buffer, "id:");
	 cmd_pos = strstr(data_buffer, "cmd:");
	 stat_pos = strstr(data_buffer, "stat:");


	 // get values & convert to CHAR
	 if (id_pos != NULL && cmd_pos != NULL && stat_pos != NULL) {
		 id = (unsigned char)atoi(id_pos + 3);
		 cmd = (unsigned char)atoi(cmd_pos + 4);
		 stat = (unsigned char)atoi(stat_pos + 5);

		 printf("[UDP->PORTAL] msg-> id: %u, cmd: %u, stat: %u\n", id, cmd, stat);
	 }
	 else {
		 printf("[UDP->PORTAL] message ERROR id: cmd: stat:   received msg INVALID!\n");
	 }


      //buffer � queue e ekle
      snprintf(message, sizeof(message), "id:%02d cmd:%02d stat:%02d", id, cmd, stat);       // addr:NULL --> ALL address  cmd:READY stat:NULL
                  //snprintf(message, sizeof(message), "id:%02d cmd:%02d stat:%02d", 16, CMD_START,CMD_NULL);       // addr:ID  cmd:START stat:NULL

      status = tx_queue_send(&Transceiver_queue_ptr, message, TX_NO_WAIT);
      if (status == TX_SUCCESS) {
        printf("\r[Portal-->Transceiver] message send: %s\n", message);
      }
#endif
      /* release the server packet */
      nx_packet_release(server_packet);

      /* toggle the green led on success */
      //HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
    }
    else
    {
      /* connection lost with the server, exit the loop */
      //break;
    }

  	//printf("\rUDP_Thread_Exit\n");

    /* Add a short timeout to let the echool tool correctly
    process the just sent packet before sending a new one */
    tx_thread_sleep(20);
  }
  /* unbind the socket and delete it */
  nx_udp_socket_unbind(&UDPSocket);
  nx_udp_socket_delete(&UDPSocket);

}
/* Send Transcevir Msg to Portal via UDP port */
VOID App_UDP_Thread_SendMESSAGE(const char *msg,unsigned char timestamp,unsigned char sensorNum)
{
  UINT ret;
  NX_PACKET *data_packet;
  CHAR message[30];

  // TX-RX thread dinleme //ardunio dinleme
  // "id:1 start.1" "id:1 start.2" .... "id:1 start.5"
  // "id:6 finish.1" ....          .... "id:6 finish.5"

  TX_MEMSET(message, '\0', sizeof(message));

  if (strcmp(msg,START_MESSAGE) == 0) {
    snprintf(message, sizeof(message), "id:%01d start.%01d", sensorNum+1, timestamp); 
  }
  else {
    snprintf(message, sizeof(message), "id:%01d finish.%01d", sensorNum+1, timestamp); 
  }

  //send message to portal
  ret = nx_packet_allocate(&NxAppPool, &data_packet, NX_UDP_PACKET, TX_WAIT_FOREVER);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  ret = nx_packet_data_append(data_packet, (VOID *)message, strlen(message), &NxAppPool, TX_WAIT_FOREVER);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* send the message */
  ret = nx_udp_socket_send(&UDPSocket, data_packet, UDP_SERVER_ADDRESS, UDP_SERVER_PORT);

  //snprintf(logmsg, sizeof(message),message);
  snprintf(logmsg, sizeof(logmsg), "%s %s %s",Module_Type[moduleType],RaceLine_State[raceState],message);
  printf("%s\r\n",logmsg);
  SENDLOG();

//printf("\r[Portal] nx_udp_socket_send ret:%d\n",ret);
  nx_packet_release(data_packet);
  
}

/**
* @brief  Link thread entry
* @param thread_input: ULONG thread parameter
* @retval none
*/
static VOID App_Link_Thread_Entry(ULONG thread_input)
{
  ULONG actual_status;
  UINT linkdown = 0, status;

  while(1)
  {
    /* Get Physical Link stackavailtus. */
    status = nx_ip_interface_status_check(&NetXDuoEthIpInstance, 0, NX_IP_LINK_ENABLED,
                                      &actual_status, 10);


    if(status == NX_SUCCESS)
    {
      if(linkdown == 1)
      {
        linkdown = 0;
        status = nx_ip_interface_status_check(&NetXDuoEthIpInstance, 0, NX_IP_ADDRESS_RESOLVED,
                                      &actual_status, 10);
        if(status == NX_SUCCESS)
        {
          /* The network cable is connected again. */
          printf("[LINK-THREAD] The network cable is connected again.\n");
          /* Print UDP Echo Client is available again. */
          printf("[LINK-THREAD] UDP Echo Client is available again.\n");
        }
        else
        {
          /* The network cable is connected. */
          printf("[LINK-THREAD] The network cable is connected.\n");
          /* Send command to Enable Nx driver. */
          nx_ip_driver_direct_command(&NetXDuoEthIpInstance, NX_LINK_ENABLE,
                                      &actual_status);
          /* Restart DHCP Client. */
          nx_dhcp_stop(&DHCPClient);
          nx_dhcp_start(&DHCPClient);
        }
      }
    }
    else
    {
      if(0 == linkdown)
      {
        linkdown = 1;
        /* The network cable is not connected. */
        printf("[LINK-THREAD] The network cable is not connected.\n");
      }
    }

    tx_thread_sleep(NX_APP_CABLE_CONNECTION_CHECK_PERIOD);
  }
}

/* Send Transceiver Msg to Portal via UDP port */
void App_UDP_Thread_Send_LOG(void)
{
  UINT ret;
  NX_PACKET *data_packet;
  CHAR message[70];

    memset(message, 0, sizeof(message));
    strcpy(message, logmsg);

    ret = nx_packet_allocate(&NxAppPool, &data_packet, NX_UDP_PACKET, TX_WAIT_FOREVER);

    if (ret != NX_SUCCESS)
    {
      Error_Handler();
    }

    ret = nx_packet_data_append(data_packet, (VOID *)message, strlen(message), &NxAppPool, TX_WAIT_FOREVER);

    if (ret != NX_SUCCESS)
    {
      Error_Handler();
    }

    /* send the message */
    ret = nx_udp_socket_send(&UDPSocket, data_packet, PRINTF_SERVER_ADDRESS, PRINTF_PORT);

    nx_packet_release(data_packet);
}

/* USER CODE END 1 */
