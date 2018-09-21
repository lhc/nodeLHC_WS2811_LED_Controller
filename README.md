# LHC Node WS2811/2812 LED Controller (ESP8266-based)

## Description

Based on nodeLHC - an ESP8266-based board with built-in USB->Serial Converter - this project implements WS2811/2812 Addressable LED controller. Full documentation about nodeLHC can be obtained here [nodeLHC Project](https://lhc.net.br/wiki/NodeLHC) and on Github [nodeLHC](https://github.com/lhc/nodeLHC). This project was built using platform.io with esp8266-nonos-sdk.

This project receives an UDP packet from user Wi-Fi AP and translate this to WS2811/WS2812 protocol format. Check Leandro's python [script](https://gist.github.com/lpereira/7178f27fe06ecfe042a0eff656786eed) that captures Webcam and send an UDP packet to a 10x10 WS2811 LED panel.  

## How to use

Clone this repository into a folder.
```
(https://github.com/lhc/nodeLHC_WS2811_LED_Controller.git)
```

### Create an ``user_config.h`` file into ``src`` folder.

This file contains user specific configuration. Check the following example for this file:
```c
#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

// User AP data
#define USER_SSID	"YOUR_WIFI_SSID"
#define USER_PASS	"YOUR_PASS"

// Configuration: UDP port and maximum number of LEDs in payload
#define MXP_UDP_PORT 2711
#define MXP_MAXLEN 300

// Uncomment the following line to set a static IP address for user Wi-Fi AP. Useful if you want to send UDP packet to an specific IP address (no broadcast).
#define SET_STATIC_IP

#ifdef SET_STATIC_IP
#define USER_AP_IP_ADDRESS		"192.168.1.10"
#define USER_AP_GATEWAY 		"192.168.1.1"
#define USER_AP_NETMASK			"255.255.255.0"
#endif

#endif
```

## UDP protocol for WS2811/WS2812 controller
Check ```mxp.h``` in ```include``` folder. 

## Editing and Building
This project was built using Visual Studio Code IDE with platform.io. Add this folder in a separate workspace and build it using platform.io (https://docs.platformio.org/en/latest/ide/vscode.html#ide-vscode). 

