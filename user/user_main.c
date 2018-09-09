/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <ets_sys.h>
#include <string.h>
#include "user_interface.h"
#include <os_type.h>
#include <gpio.h>
#include "espconn.h"
#include "../include/driver/uart.h"
#include "../include/ws2811dma.h"
#include "../include/mxp.h"
#include "../include/user_config.h"

static struct ip_info ipConfig;
unsigned char *p = (unsigned char*)&ipConfig.ip.addr;

/* User AP config is defined in wifi_config.h, make sure to create this file and associate USER_SSID and USER_PASS*. This file will not be versioned.*/
#define WIFI_CLIENTSSID			USER_SSID
#define WIFI_CLIENTPASSWORD		USER_PASS

extern int ets_uart_printf(const char *fmt, ...);

void ICACHE_FLASH_ATTR wifiConnectCb(System_Event_t *evt)
{
	ets_uart_printf("Wifi event: %d\r\n", evt->event);
	switch (evt->event) {
	case EVENT_STAMODE_CONNECTED:
		ets_uart_printf("connected to ssid %s, channel %d\n",
				evt->event_info.connected.ssid,
				evt->event_info.connected.channel);

		break;
	case EVENT_STAMODE_DISCONNECTED:
		ets_uart_printf("disconnected from ssid %s, reason %d\n",
				evt->event_info.disconnected.ssid,
				evt->event_info.disconnected.reason);
		break;
	case EVENT_STAMODE_AUTHMODE_CHANGE:
		ets_uart_printf("mode: %d -> %d\n",
				evt->event_info.auth_change.old_mode,
				evt->event_info.auth_change.new_mode);
		break;
	case EVENT_STAMODE_GOT_IP:
		wifi_get_ip_info(STATION_IF, &ipConfig);
		ets_uart_printf("%d.%d.%d.%d\n\nListening to UDP packages for LED display...",p[0],p[1],p[2],p[3]);
		mxp_init(ws2811dma_put);
		break;
	case EVENT_SOFTAPMODE_STACONNECTED:
		ets_uart_printf("station: " MACSTR "join, AID = %d\n",
				MAC2STR(evt->event_info.sta_connected.mac),
				evt->event_info.sta_connected.aid);
		break;
	case EVENT_SOFTAPMODE_STADISCONNECTED:
		ets_uart_printf("station: " MACSTR "leave, AID = %d\n",
				MAC2STR(evt->event_info.sta_disconnected.mac),
				evt->event_info.sta_disconnected.aid);
		break;
	default:
		break;
	}
}

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABBBCDDD
 *                A : rf cal
 *                B : at parameters
 *                C : rf init data
 *                D : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 8;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void user_rf_pre_init(void){

}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
	// Configure the UART
	uart_init(BIT_RATE_115200, BIT_RATE_115200);

	espconn_init();
	ets_uart_printf("SDK version:%s\r\n", system_get_sdk_version());
	static struct station_config stconfig;

	wifi_set_opmode(STATION_MODE);
	wifi_station_disconnect();
	wifi_station_dhcpc_stop();
	wifi_station_set_auto_connect(0);
	if(wifi_station_get_config(&stconfig))
	{
		memcpy(&stconfig.ssid, WIFI_CLIENTSSID, sizeof(WIFI_CLIENTSSID));
		memcpy(&stconfig.password, WIFI_CLIENTPASSWORD, sizeof(WIFI_CLIENTPASSWORD));
		wifi_station_set_config(&stconfig);
		ets_uart_printf("SSID: %s\n",stconfig.ssid);
	}

#ifdef SET_STATIC_IP
	ipConfig.ip.addr = ipaddr_addr(USER_AP_IP_ADDRESS);
	ipConfig.gw.addr = ipaddr_addr(USER_AP_GATEWAY);
	ipConfig.netmask.addr = ipaddr_addr(USER_AP_NETMASK);
#endif

	wifi_set_ip_info(STATION_IF, &ipConfig);
	wifi_station_set_auto_connect(1);
	wifi_station_connect();
	ets_uart_printf("Hello World\n\n");
	wifi_set_event_handler_cb(wifiConnectCb);

	ets_uart_printf("Initializing WS2811...\n\n");
	ws2811dma_init();
}
