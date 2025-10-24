#pragma once

// #ifdef U8X8_HAVE_HW_SPI
// #include <SPI.h>
// #endif
// #ifdef U8X8_HAVE_HW_I2C
// #include <Wire.h>
// #endif

// Built-in LED for ESP32
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// DCF77 receiver settings
#define DBG 0
#define BLINK 0
#define INP 26

// GPS receiver settings
#define GPS_UART 2

// display parameters
#define DISP_PERIOD_MS 500

// time sync parameters
#define SYNC_PERIOD_S 120

// max log message length
#define MAX_LOG_MSG_LEN 80
