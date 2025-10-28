#pragma once


// Built-in LED for ESP32
#ifndef LED_BUILTIN
#define LED_BUILTIN 2   // GPIO2 is used for built-in LED on most ESP32 module
#endif

// DCF77 receiver settings
#define DBG 0         // serial debug messages of all received bits
#define DCF_BLINK 1   // bit reception monitor on LED
#define INP_PIN 26    // input pin for signal of DCF77 receiver module
#define DCF_TZ "CET-1CEST,M3.5.0/2,M10.5.0/3"   // timezone for DCF77 receiver

// GPS receiver settings
#define GPS_UART 2        // hardware UART number where GPS module is connected to
#define GPS_RX_PIN 16     // GPIO pin mapped to RX
#define GPS_TX_PIN 17     // GPIO pin mapped to TX
#define GPS_BAUD 115200   // serial communication bitspeed of GPS module

// display parameters
#define DISP_PERIOD_MS 500    // screen refresh period in milliseconds
#define DISP_TZ "CET-1CEST,M3.5.0,M10.5.0/3"   // timezone for local time

// time sync parameters
#define SYNC_PERIOD_S 120     // time considered as synchronized in seconds from last sync

// max log message length
#define MAX_LOG_QUE_LEN 10    // log queue length (number of maximum messages in queue)
#define MAX_LOG_MSG_LEN 80    // maximum length of a log message
#define LOG_SPEED 115200      // serial logging port speed
