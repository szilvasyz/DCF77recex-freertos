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

struct TzRule DCFrule;
struct TzRule GPSrule;
struct TzRule DISPrule;

time_t lastDCFtime = 0;
time_t lastGPStime = 0;
time_t lastsynced = 0;


void setup() {

  setenv("TZ", "UTC0", 1);
  tzset();

  struct tm timeinfo = makeTm(2020, 1, 1, 0, 0, 0);
  time_t now = mktime(&timeinfo);  // Unix időbélyeg
  struct timeval tv = { now, 0 };
  settimeofday(&tv, nullptr);  // beállítja a rendszeridőt

  last = millis();

  parseTzString(DCF_TZ, DCFrule);
  parseTzString(GPS_TZ, GPSrule);
  parseTzString(DISP_TZ, DISPrule);

  tzMutex = xSemaphoreCreateMutex();
  u8g2Mutex = xSemaphoreCreateMutex();
  logQueue = xQueueCreate(MAX_LOG_QUE_LEN, MAX_LOG_MSG_LEN);

  initLogger();
  initScr();
  initDcf();
  initGps();

  #if (DCF_BLINK == 0)
    xTaskCreate(blinkTask, "Blink", 2048, NULL, 0, NULL);
  #endif
  xTaskCreate(loggerTask, "Logger", 2048, NULL, 1, NULL);
  xTaskCreate(scrTask, "Screen refresh", 2048, NULL, 1, NULL);
  xTaskCreate(dcfTask, "DCF receiver", 2048, NULL, 3, NULL);
  xTaskCreate(gpsTask, "GPS receiver", 2048, NULL, 2, NULL);
  xTaskCreate(tzWatchdogTask, "TZ watchdog", 2048, NULL, 1, NULL);
  #if (GPS_COPY != 0)
    xTaskCreate(gpsCopyTask, "GPS receiver", 2048, NULL, 2, NULL);
  #endif

}


void loop() {

  vTaskDelay(pdMS_TO_TICKS(DISP_PERIOD_MS));

}
