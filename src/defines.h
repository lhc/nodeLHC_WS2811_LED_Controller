#ifndef _DEFINES_H_
#define _DEFINES_H_

//__ Dines all the protocol availables
#define PROTOCOL_ARTNET 0
#define PROTOCOL_MXP    1

//__ Default Protocol selected
#ifndef PROTOCOL_SELECTED
    #define PROTOCOL_SELECTED PROTOCOL_ARTNET
#endif

#ifndef BRIGHTNESS
    //__ Set brightness of all leds of the panel
    #define BRIGHTNESS 100
#endif

//__ The DEBUG enable, change some values
#ifdef DEBUG
    //__ Set brightness to 25%
    #undef BRIGHTNESS
    #define BRIGHTNESS 25
#endif



////////////////////////////////////////////////////////////////
// SANE CHECKS
#if BRIGHTNESS > 100
    #error BRIGHTNESS must be a number between 0 and 100
#endif


#endif //_DEFINES_H_