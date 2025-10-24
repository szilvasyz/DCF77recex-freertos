#include "common.h"
#include "blink.h"
#include "logger.h"
#include "screen.h"
#include "dcfread.h"


HardwareSerial GPS(GPS_UART);  // UART2: GPIO16 (RX2), GPIO17 (TX2)
DCFrec receiver(INP, DBG);
U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 5, /* dc=*/13, /* reset=*/ U8X8_PIN_NONE);  

QueueHandle_t logQueue;
SemaphoreHandle_t u8g2Mutex; // Mutex

char buf[MAX_LOG_MSG_LEN];
const char* DWS[] = {"-0-", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
unsigned long curr, last;
int heartbeat = 0;
time_t lastsynced = 0;


void setup() {

  #if (BLINK != 0)
    receiver.begin(LED_BUILTIN);
  #else
    receiver.begin(PIN_NONE);
  #endif

  WiFi.mode(WIFI_OFF);
  btStop();
  Serial.begin(115200);
  GPS.begin(115200, SERIAL_8N1); // baud megegyezik a moduléval

  u8g2.begin();
  //u8g2.setFont(u8g2_font_rosencrantz_nbp_tf);	// choose a suitable font
  //u8g2.setFont(u8g2_font_profont11_mf);	// choose a suitable font
  u8g2.setFont(u8g2_font_7x14_mf);	// choose a suitable font
   
  u8g2.setFontPosTop();
  u8g2.setFontMode(0);

  u8g2.setCursor(0, 48);
  u8g2.print("Starting...234567890");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display


  struct tm t = {0};

  // példa: 2020. január 1. 00:00:00
  t.tm_year = 2010 - 1900;  // év - 1900
  t.tm_mon  = 1 - 1;        // hónap (0–11)
  t.tm_mday = 1;            // nap
  t.tm_hour = 0;
  t.tm_min  = 0;
  t.tm_sec  = 0;

  time_t now = mktime(&t);  // Unix időbélyeg
  struct timeval tv = { now, 0 };
  settimeofday(&tv, nullptr);  // beállítja a rendszeridőt

 
  last = millis();

  u8g2Mutex = xSemaphoreCreateMutex();
  logQueue = xQueueCreate(10, MAX_LOG_MSG_LEN);


  xTaskCreate(blinkTask, "Blink", 2048, NULL, 1, NULL);
  xTaskCreate(loggerTask, "Logger", 2048, NULL, 1, NULL);
  xTaskCreate(dcfTask, "DCF receiver", 2048, NULL, 1, NULL);
  xTaskCreate(scrTask, "Screen refresh", 2048, NULL, 1, NULL);

}


void loop() {

  vTaskDelay(pdMS_TO_TICKS(DISP_PERIOD_MS));
  // if (((curr=millis()) - last) >= DISP_PERIOD) {
  //   last = curr;

    struct tm timeinfo;
    getLocalTime(&timeinfo, 20);  // lekérdezzük az aktuális időt (fut az ESP32 RTC-je)
    strftime(buf, sizeof(buf), "%a %y-%m-%d %H:%M", &timeinfo);

    xSemaphoreTake(u8g2Mutex, portMAX_DELAY);
    u8g2.setCursor(0, 16);
    u8g2.print(buf);	// write something to the internal memory

    strftime(buf, sizeof(buf), "%a %y-%m-%d %H:%M", localtime(&lastsynced));
    u8g2.setCursor(0, 48);
    u8g2.print(buf);	// write something to the internal memory
    xSemaphoreGive(u8g2Mutex);

    //u8g2.sendBuffer();					// transfer internal memory to the display
  // }

}
