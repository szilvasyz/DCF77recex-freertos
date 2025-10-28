#include "common.h"
#include "blink.h"
#include "logger.h"
#include "screen.h"
#include "sysclk.h"
#include "dcfread.h"
#include "gpsread.h"
#include "tzwatchdog.h"


HardwareSerial GPS(GPS_UART);  // UART2: GPIO16 (RX2), GPIO17 (TX2)
DCFrec receiver(INP_PIN, DBG);
U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 5, /* dc=*/13, /* reset=*/ U8X8_PIN_NONE);  

QueueHandle_t logQueue;
SemaphoreHandle_t u8g2Mutex; // Mutex
SemaphoreHandle_t tzMutex; // Mutex

char buf[MAX_LOG_MSG_LEN];
const char* DWS[] = {"-0-", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
unsigned long curr, last;
int heartbeat = 0;
time_t lastsynced = 0;

time_t lastDCFtime = 0;
time_t lastGPStime = 0;


void setup() {

  // WiFi.mode(WIFI_OFF);
  // btStop();

  tzMutex = xSemaphoreCreateMutex();
  u8g2Mutex = xSemaphoreCreateMutex();
  logQueue = xQueueCreate(MAX_LOG_QUE_LEN, MAX_LOG_MSG_LEN);

  initSysClk();
  initLogger();
  initScr();
  initDcf();
  initGps();

  //xTaskCreate(blinkTask, "Blink", 2048, NULL, 0, NULL);
  xTaskCreate(loggerTask, "Logger", 2048, NULL, 1, NULL);
  xTaskCreate(scrTask, "Screen refresh", 2048, NULL, 1, NULL);
  xTaskCreate(dcfTask, "DCF receiver", 2048, NULL, 3, NULL);
  xTaskCreate(gpsTask, "GPS receiver", 2048, NULL, 2, NULL);
  xTaskCreate(tzWatchdogTask, "TZ watchdog", 2048, NULL, 1, NULL);
  //xTaskCreate(gpsCopyTask, "GPS receiver", 2048, NULL, 2, NULL);

}


void loop() {

  vTaskDelay(pdMS_TO_TICKS(DISP_PERIOD_MS));
  // if (((curr=millis()) - last) >= DISP_PERIOD) {
  //   last = curr;

    // struct tm timeinfo;
    // time_t now = time(nullptr);
    // localtime_r(&now, &timeinfo);
    // //getLocalTime(&timeinfo, 20);  // lekérdezzük az aktuális időt (fut az ESP32 RTC-je)
    // strftime(buf, sizeof(buf), "%a %y-%m-%d %H:%M", &timeinfo);

    // xSemaphoreTake(u8g2Mutex, portMAX_DELAY);
    // u8g2.setCursor(0, 16);
    // u8g2.print(buf);	// write something to the internal memory

    // strftime(buf, sizeof(buf), "%a %y-%m-%d %H:%M", localtime_r(&lastDCFtime, &timeinfo));
    // u8g2.setCursor(0, 32);
    // u8g2.print(buf);	// write something to the internal memory

    // strftime(buf, sizeof(buf), "%a %y-%m-%d %H:%M", localtime_r(&lastGPStime, &timeinfo));
    // u8g2.setCursor(0, 48);
    // u8g2.print(buf);	// write something to the internal memory

    // xSemaphoreGive(u8g2Mutex);

    //u8g2.sendBuffer();					// transfer internal memory to the display
  // }

}
