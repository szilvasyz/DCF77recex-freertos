//#include <ctime>
#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
//#include <WiFi.h>
#include <U8g2lib.h>
#include <dcfrec.h>
#include <time.h>

#include "config.h"


extern HardwareSerial GPS;
extern DCFrec receiver;
extern U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2;

extern QueueHandle_t logQueue;
extern SemaphoreHandle_t u8g2Mutex;
extern SemaphoreHandle_t tzMutex; // Mutex

extern char buf[MAX_LOG_MSG_LEN];
extern const char* DWS[];
extern unsigned long curr, last;
extern int heartbeat;

extern struct TzRule DCFrule;
extern struct TzRule GPSrule;
extern struct TzRule DISPrule;

extern time_t lastDCFtime;
extern time_t lastGPStime;
extern time_t lastsynced;

static inline struct tm makeTm(int year, int month, int day,
                               int hour, int minute, int second = 0)
{
    return {
        .tm_sec   = second,
        .tm_min   = minute,
        .tm_hour  = hour,
        .tm_mday  = day,
        .tm_mon   = month - 1,
        .tm_year  = year - 1900,
        .tm_isdst = -1
    };
}
