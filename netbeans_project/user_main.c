#include <stdio.h>

#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "uart.h"
#include "gpio.h"
#include "user_config.h"
#ifndef LWIP_DEBUG
#include "espconn.h"
#endif

static ETSTimer BtnTimer;
struct espconn Conn;
static ETSTimer WiFiLinker;
//esp_tcp ConnTcp;
esp_udp ConnUdp;

static void wifi_check_ip(void *arg);

extern void ets_wdt_enable (void);
extern void ets_wdt_disable (void);
extern void wdt_feed (void);

static void ICACHE_FLASH_ATTR tcpclient_sent_cb(void *arg)
{
    struct espconn *pespconn = arg;
    
    #ifdef DEBUG_SERIAL
    os_printf("tcpclient_sent_cb\r\n");
    #endif

    espconn_disconnect(pespconn);
}

static void ICACHE_FLASH_ATTR tcpclient_discon_cb(void *arg)
{
    struct espconn *pespconn = arg;

    #ifdef DEBUG_SERIAL
    os_printf("tcpclient_discon_cb\r\n");
    #endif

    if (pespconn == NULL)
    {
        #ifdef DEBUG_SERIAL
        os_printf("tcpclient_discon_cb - conn is NULL\r\n");
        #endif
        return;
    }
}

static void ICACHE_FLASH_ATTR tcpclient_connect_cb(void *arg)
{
    struct espconn *pespconn = arg;
    char payload[128];
    
    #ifdef DEBUG_SERIAL
    os_printf("tcpclient_connect_cb\r\n");
    #endif

    espconn_regist_sentcb(pespconn, tcpclient_sent_cb);

    os_sprintf(payload, "ESP8266 >>>>>>>>");
    sint8 espsent_status = espconn_sent(pespconn, payload, strlen(payload));
}

static void ICACHE_FLASH_ATTR tcpclient_recon_cb(void *arg, sint8 err)
{
    struct espconn *pespconn = arg;

    //os_timer_disarm(&WiFiLinker);
    //os_timer_setfn(&WiFiLinker, (os_timer_func_t *)platform_reconnect, pespconn);
    //os_timer_arm(&WiFiLinker, 2000, 0);
    
    #ifdef DEBUG_SERIAL
    os_printf("tcpclient_recon_cb\r\n");
    #endif
}

void ICACHE_FLASH_ATTR udp_sent_data(void *arg)
{
    os_printf("udp_sent_data\r\n");
}

void ICACHE_FLASH_ATTR udp_recv_data(void *arg, char *pdata, unsigned short len)
{
    struct espconn *pespconn = (struct espconn *)arg;
    char buf_recv[20];
    
    strncpy(buf_recv,pdata,len);
    os_printf("udp_recv_data\r\n");
    os_printf(buf_recv);
    os_printf("\r\n");
    
/*
  at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;
  char temp[32];

  os_printf("recv\r\n");
  if(pespconn->changType == 0) //if when sending, receive data???
  {
    os_memcpy(pespconn->proto.udp->remote_ip, linkTemp->remoteIp, 4);
    pespconn->proto.udp->remote_port = linkTemp->remotePort;
  }
  else if(linkTemp->changType == 1)
  {
    os_memcpy(linkTemp->remoteIp, pespconn->proto.udp->remote_ip, 4);
    linkTemp->remotePort = pespconn->proto.udp->remote_port;
    linkTemp->changType = 0;
  }
//  else if(linkTemp->changType == 2)
//  {
//    os_memcpy(linkTemp->remoteIp, pespconn->proto.udp->remote_ip, 4);
//    linkTemp->remotePort = pespconn->proto.udp->remote_port;
//  }

  if(at_ipMux)
  {
    os_sprintf(temp, "\r\n+IPD,%d,%d:",
               linkTemp->linkId, len);
    uart0_sendStr(temp);
    uart0_tx_buffer(pdata, len);
  }
  else if(IPMODE == FALSE)
  {
    os_sprintf(temp, "\r\n+IPD,%d:", len);
    uart0_sendStr(temp);
    uart0_tx_buffer(pdata, len);
  }
  else
  {
    uart0_tx_buffer(pdata, len);
    return;
  }
  at_backOk;
*/
}

static void ICACHE_FLASH_ATTR senddata(void)
{
    char tcpserverip[15];
    char data_sent[20];
    
    os_timer_disarm(&WiFiLinker);
	
    Conn.proto.udp = &ConnUdp;
    Conn.type = ESPCONN_UDP;
    Conn.state = ESPCONN_NONE;
    
    os_sprintf(tcpserverip, "%s", UDP_BROADCAST_IP);
    uint32_t ip = ipaddr_addr(tcpserverip);
    
    os_memcpy(Conn.proto.udp->remote_ip, &ip, 4);
    Conn.proto.udp->local_port = UDP_DEFAULT_PORT;
    Conn.proto.udp->remote_port = UDP_DEFAULT_PORT;
    
    //espconn_regist_connectcb(&Conn, tcpclient_connect_cb);
    //espconn_regist_reconcb(&Conn, tcpclient_recon_cb);
    //espconn_regist_disconcb(&Conn, tcpclient_discon_cb);

    espconn_regist_recvcb(&Conn, udp_recv_data);
    espconn_regist_sentcb(&Conn, udp_sent_data);
    
    wifi_set_broadcast_if(1);
    
    //sint8 espcon_status = espconn_connect(&Conn);
    sint8 espcon_status = espconn_create(&Conn);
    sprintf(data_sent,"DADOS >>>>>>");
    espconn_sent(&Conn,data_sent,strlen(data_sent));
    espcon_status=0;
    
/*
    #ifdef DEBUG_SERIAL
    switch(espcon_status)
    {
        case ESPCONN_OK:
            os_printf("TCP created.\r\n");
            break;
        case ESPCONN_RTE:
            os_printf("Error connection, routing problem.\r\n");
            break;
        case ESPCONN_TIMEOUT:
            os_printf("Error connection, timeout.\r\n");
            break;
        default:
            os_printf("Connection error: %d\r\n", espcon_status);
    }
    #endif

    if(espcon_status != ESPCONN_OK)
    {
        os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
        os_timer_arm(&WiFiLinker, 1000, 0);
    }
*/
}

static void ICACHE_FLASH_ATTR wifi_check_ip(void *arg)
{
    struct ip_info ipConfig;
    int state_;

    os_timer_disarm(&WiFiLinker);
    
    state_ = wifi_station_get_connect_status();
    
    #ifdef DEBUG_SERIAL
    os_printf("wifi_check_ip\r\n");
    #endif
    
    if(state_ == STATION_GOT_IP)
    {
        wifi_get_ip_info(STATION_IF, &ipConfig);
        if(ipConfig.ip.addr != 0)
        {
            #ifdef DEBUG_SERIAL
            os_printf("WiFi connected\r\n");
            #endif
        }
    }
    
    //os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
    //os_timer_arm(&WiFiLinker, 1000, 0);
}

static void ICACHE_FLASH_ATTR BtnTimerCb(void *arg)
{
    static int cnt=0;
    
    //re
    
    //GPIO2
    if (!GPIO_INPUT_GET(2))
    {
        cnt++;
    }
    else
    {
        if (cnt != 0)
        {
            #ifdef DEBUG_SERIAL
            os_printf("GPIO2 PRECIONADO\r\n");
            #endif
            //send_udp_data();
            senddata();
        }
        cnt = 0;
    }
}

//funcao principal
void user_init(void)
{
    char temp[100];
    
    //inicializa SERIAL DEBUG
    #ifdef DEBUG_SERIAL
    uart0_init();
    os_delay_us(1000);
    #endif
    
    ets_wdt_enable();
    ets_wdt_disable();
    
    uint32 ttt;
    ttt=5000;
    while(ttt--)
    {
        os_delay_us(1000);
        wdt_feed();
    }
    
    #ifdef DEBUG_SERIAL
    os_printf("INICIO...\r\n");
    #endif
    
    //inicia o timer zerado
    system_timer_reinit();

    wifi_set_opmode((wifi_get_opmode()|STATION_MODE)&STATION_MODE);
    
    struct station_config stconfig;
    
    wifi_station_disconnect();
    wifi_station_dhcpc_stop();
    
    if(wifi_station_get_config(&stconfig))
    {
        os_memset(stconfig.ssid, 0, sizeof(stconfig.ssid));
        os_memset(stconfig.password, 0, sizeof(stconfig.password));
        os_sprintf(stconfig.ssid, "%s", WIFI_CLIENTSSID);
        os_sprintf(stconfig.password, "%s", WIFI_CLIENTPASSWORD);
            
		#ifdef DEBUG_SERIAL
		os_printf("wifi_station_get_config\r\n");
		#endif
			
        if(!wifi_station_set_config(&stconfig))
        {
            #ifdef DEBUG_SERIAL
            os_printf("wifi_station_set_config\r\n");
            #endif
        }
    }
    
    wifi_station_connect();
    wifi_station_dhcpc_start();
    //wifi_station_set_auto_connect(1);
    
    #ifdef DEBUG_SERIAL
    os_printf("ESP8266 in STA mode configured.\r\n");
    #endif

    if(wifi_get_phy_mode() != PHY_MODE_11N)
    {
        #ifdef DEBUG_SERIAL
        os_printf("wifi_set_phy_mode\r\n");
        #endif
        wifi_set_phy_mode(PHY_MODE_11N);
    }
    if(wifi_station_get_auto_connect() == 0)
    {
        #ifdef DEBUG_SERIAL
        os_printf("wifi_station_set_auto_connect\r\n");
        #endif
        wifi_station_set_auto_connect(1);
    }

    // Select pin function GPIO2
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    // Disable pulldown GPIO2
    PIN_PULLDWN_DIS(PERIPHS_IO_MUX_GPIO2_U);
    // Enable pull up GPIO2
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);  
    // Set GPIO2 as input mode
    gpio_output_set(0, 0, 0, BIT2);
    os_timer_disarm(&BtnTimer);
    os_timer_setfn(&BtnTimer, BtnTimerCb, NULL);
    os_timer_arm(&BtnTimer, 500, 1);
    
    // Wait for Wi-Fi connection and start TCP connection
    os_timer_disarm(&WiFiLinker);
    os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
    os_timer_arm(&WiFiLinker, 1000, 0);

    #ifdef DEBUG_SERIAL
    os_printf("ESP8266 CONFIGURED\r\n");
    #endif
}
