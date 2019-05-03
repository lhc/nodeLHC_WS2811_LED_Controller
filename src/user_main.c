#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include <esp_wifi.h>
#include <uart.h>
#include "defines.h"

#include "user_config.h"
#if PROTOCOL_SELECTED == PROTOCOL_MXP
    #include "mxp.h"
#elif PROTOCOL_SELECTED == PROTOCOL_ARTNET
    #include "artnet.h"
#endif
#include "ws2811dma.h"

#ifdef FontIP
    #include <fontToMatrix.h>
#endif


/**
 * Caso a placa fique piscando loucamente após gravar e não volte a funcionar nem por ação de reza brava. Use o seguinte comando
 *   esptool.py --port /dev/ttyUSB0 write_flash 0x3fc000 ~/.platformio/packages/framework-esp8266-rtos-sdk/bin/esp_init_data_default.bin
 * 
 * Para instalar o esptool.py, use
 *   pip install esptool
 * 
 * Para dar erase 
 *   esptool.py --port /dev/ttyUSB0 erase_flash
 * 
 */

static struct ip_info ipConfig = { .ip = 0xffffffff, .netmask = 0xffffffff, .gw = 0xffffffff};
unsigned char *p = (unsigned char*)&ipConfig.ip.addr;

/* User AP config is defined in wifi_config.h, make sure to create this file and associate USER_SSID and USER_PASS*. This file will not be versioned.*/
#define WIFI_CLIENTSSID            USER_SSID
#define WIFI_CLIENTPASSWORD        USER_PASS

/* Prototypes */
void showIP(uint8_t ip[4]);

void ICACHE_FLASH_ATTR wifiConnectCb(System_Event_t *evt)
{
    printf("Wifi event: %d\r\n", evt->event_id);
    switch (evt->event_id) {
    case EVENT_STAMODE_CONNECTED:
        printf("connected to ssid \"%s\", channel %d\n",
                evt->event_info.connected.ssid,
                evt->event_info.connected.channel);

        break;
    case EVENT_STAMODE_DISCONNECTED:
        printf("disconnected from ssid \"%s\", reason %d\n",
                evt->event_info.disconnected.ssid,
                evt->event_info.disconnected.reason);
        break;
    case EVENT_STAMODE_AUTHMODE_CHANGE:
        printf("mode: %d -> %d\n",
                evt->event_info.auth_change.old_mode,
                evt->event_info.auth_change.new_mode);
        break;
    case EVENT_STAMODE_GOT_IP:
        wifi_get_ip_info(STATION_IF, &ipConfig);
        printf("Device IP: %d.%d.%d.%d\n\nListening to UDP packages for LED display...\n",p[0],p[1],p[2],p[3]);
        #if PROTOCOL_SELECTED == PROTOCOL_MXP
            printf("Running MXP!\n");
            mxp_init(ws2811dma_put);
        #elif PROTOCOL_SELECTED == PROTOCOL_ARTNET
            printf("Running Artnet!\n");
            artnet_init();
        #endif
        

        #ifndef FontIP
            showIP(p);
        #endif

        break;
    case EVENT_SOFTAPMODE_STACONNECTED:
        printf("station: " MACSTR "join, AID = %d\n",
                MAC2STR(evt->event_info.sta_connected.mac),
                evt->event_info.sta_connected.aid);
        break;
    case EVENT_SOFTAPMODE_STADISCONNECTED:
        printf("station: " MACSTR "leave, AID = %d\n",
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
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
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
        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}



void task_wait(void* ignore)
{
    #define PIN 2
    GPIO_AS_OUTPUT(PIN);
    uint32_t state = 0;
    uint8_t data[MAXPIXELS][3] = {0};
    char test_str[22]; // Maximum size of the IP format string (xxx.xxx.xxx.xxx:xxxxx)
    vTaskDelay(1000/portTICK_RATE_MS);
    while(true) {
        #ifdef FontIP
            #if PROTOCOL_SELECTED == PROTOCOL_MXP
                if (mxp_is_active() == 0){
            #elif PROTOCOL_SELECTED == PROTOCOL_ARTNET
                if (artnet_is_active() == 0){
            #endif
                GPIO_OUTPUT_SET(PIN, 0);
                vTaskDelay(50/portTICK_RATE_MS);
                //__ Test if it get a valid IP
                if ( (p[0] != 0xff) && (p[1] != 0xff) && (p[2] != 0xff) && (p[3] != 0xff) ){
                    sprintf(test_str, "%d.%d.%d.%d:%d", p[0], p[1], p[2], p[3], ARTNET_Port);
                    strToFrame((char *)test_str, &state, (uint8_t *)data);
                    ws2811dma_put((uint8_t *)&data[0], MAXPIXELS, 0);
                }
                GPIO_OUTPUT_SET(PIN, 1);
                vTaskDelay(50/portTICK_RATE_MS);
            } else 
        #endif
        {
            GPIO_OUTPUT_SET(PIN, 0);
            vTaskDelay(100/portTICK_RATE_MS);
            GPIO_OUTPUT_SET(PIN, 1);
            vTaskDelay(100/portTICK_RATE_MS);
        }
    }

    vTaskDelete(NULL);
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    uart_init_new();
	UART_SetBaudrate(UART0, 115200);
	UART_SetBaudrate(UART1, 115200);
    xTaskCreate(&task_wait, "wait", 2048, NULL, 1, NULL);

    espconn_init();
    printf("SDK version:%s\r\n", system_get_sdk_version());
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
        printf("SSID: %s\n",stconfig.ssid);
    }

#ifdef SET_STATIC_IP
    ipConfig.ip.addr = ipaddr_addr(USER_AP_IP_ADDRESS);
    ipConfig.gw.addr = ipaddr_addr(USER_AP_GATEWAY);
    ipConfig.netmask.addr = ipaddr_addr(USER_AP_NETMASK);
#else
	wifi_station_dhcpc_start();
#endif


    wifi_set_ip_info(STATION_IF, &ipConfig);
    wifi_station_set_auto_connect(1);
    wifi_station_connect();
    printf("\"Laboratorio Hacker de Campinas\"\n\tLED Panel\n");
    #ifdef DEBUG
        printf("\n@@@@@@@@@@@@@@ DEBUG ENABLE @@@@@@@@@@@@@@\n\n");
        printf("The brigtness is set to %d%\n", BRIGHTNESS);
        printf("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n");
    #endif
    wifi_set_event_handler_cb(wifiConnectCb);

    printf("Initializing WS2811...\n\n");
    ws2811dma_init();

}

void showIP(uint8_t ip[4]){
    uint8_t data[MAXPIXELS][3] = {0};
    ws2811dma_put((uint8_t *)&data[0], MAXPIXELS, 0);

    printf("%d.%d.%d.%d\n",p[0],p[1],p[2],p[3]);

    for(int i = 0; i < 1; i++){
        for(int j = 0; j < 8; j++){
            if (ip[0] & (0x01 << j)){
                data[8*i+(7-j)][0] = 255;
                data[8*i+(7-j)][1] = 255;
                data[8*i+(7-j)][2] = 255;
            }
        }
    }

    for(int i = 2; i < 3; i++){
        for(int j = 0; j < 8; j++){
            if (ip[1] & (0x01 << j)){
                data[8*i+(7-j)][0] = 255;
                data[8*i+(7-j)][1] = 255;
                data[8*i+(7-j)][2] = 255;
            }
        }
    }

    for(int i = 4; i < 5; i++){
        for(int j = 0; j < 8; j++){
            if (ip[2] & (0x01 << j)){
                data[8*i+(7-j)][0] = 255;
                data[8*i+(7-j)][1] = 255;
                data[8*i+(7-j)][2] = 255;
            }
        }
    }

    for(int i = 6; i < 7; i++){
        for(int j = 0; j < 8; j++){
            if (ip[3] & (0x01 << j)){
                data[8*i+(7-j)][0] = 255;
                data[8*i+(7-j)][1] = 255;
                data[8*i+(7-j)][2] = 255;
            }
        }
    }

    ws2811dma_put((uint8_t *)&data[0], MAXPIXELS, 0);
}
