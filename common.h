#pragma once

#include <WiFi.h>
#include <U8g2lib.h>
#include <dcfrec.h>
#include <time.h>

#include "config.h"


extern HardwareSerial GPS;
extern DCFrec receiver;
extern U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2;

extern QueueHandle_t logQueue;
extern SemaphoreHandle_t u8g2Mutex;

extern char buf[MAX_LOG_MSG_LEN];
extern const char* DWS[];
extern unsigned long curr, last;
extern int heartbeat;
extern time_t lastsynced;


