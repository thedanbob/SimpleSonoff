/*
  ===================================================================================================
    DON'T CHANGE ANYTHING BELOW UNLESS YOU KNOW WHAT YOU'RE DOING. UPDATE YOUR SETTINGS IN CONFIG.H
  ===================================================================================================
*/

#ifndef SIMPLE_SONOFF_DEFINES_H
#define SIMPLE_SONOFF_DEFINES_H

#include "config.h"

#ifdef MULTI
#define CHANNELS 4
#else
#define CHANNELS 1
#endif

#define LED_PIN 13
#define OPT_PIN 14
#define BTN_PIN_1 0
#define BTN_PIN_2 9
#define BTN_PIN_3 10
#define BTN_PIN_4 14
#define RELAY_PIN_1 12
#define RELAY_PIN_2 5
#define RELAY_PIN_3 4
#define RELAY_PIN_4 15

#endif
