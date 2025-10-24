#include <WiFi.h>
#include <U8g2lib.h>
#include <dcfrec.h>
#include <time.h>


#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#define DBG 0
#define BLINK 0
//#define INP 5
#define INP 26
#define GPS_UART 2


#define DISP_PERIOD 500
#define SYNC_PERIOD 120

//extern "C" void btStop(void);


HardwareSerial GPS(GPS_UART);  // UART2: GPIO16 (RX2), GPIO17 (TX2)
DCFrec receiver(INP, DBG);
U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 5, /* dc=*/13, /* reset=*/ U8X8_PIN_NONE);  


#define MAX_MSG_LEN 80
QueueHandle_t logQueue = xQueueCreate(10, MAX_MSG_LEN);

static char buf[MAX_MSG_LEN];
const char* DWS[] = {"-0-", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
static unsigned long curr, last;
static int heartbeat = 0;


time_t lastsynced = 0;

SemaphoreHandle_t u8g2Mutex; // Mutex


void blinkTask(void* pvParameters) {
  pinMode(LED_BUILTIN, OUTPUT);
  for (;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1000));
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


void loggerTask(void* pvParameters) {
  char msg[MAX_MSG_LEN];
  for (;;) {
    if (xQueueReceive(logQueue, msg, portMAX_DELAY)) {
      Serial.println(msg);
    }
  }
}


void scrTask(void* pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(DISP_PERIOD);

  for (;;) {
    vTaskDelayUntil(&lastWakeTime, period);  
  
    heartbeat = !heartbeat;
    time_t t = time(nullptr);

    xSemaphoreTake(u8g2Mutex, portMAX_DELAY);
    u8g2.setCursor(0,0);
    u8g2.print("*O"[heartbeat]);
    u8g2.print(" S"[(t - lastsynced) < SYNC_PERIOD]);
    xSemaphoreGive(u8g2Mutex);

    u8g2.sendBuffer();
  }
}


void dcfTask(void* pvParameters) {
  DCFtime mytime;

  while(true) {
  
    receiver.getTime(&mytime);

    if (mytime.newtime) {
      struct tm t = {0};

      t.tm_year = mytime.year + 100;  // év - 1900
      t.tm_mon  = mytime.month - 1;   // hónap (0–11)
      t.tm_mday = mytime.day;         // nap
      t.tm_hour = mytime.hour;
      t.tm_min  = mytime.minute;
      t.tm_sec  = 0;

      time_t now = mktime(&t);  // Unix időbélyeg
      lastsynced = now;
      struct timeval tv = { now, 0 };
      settimeofday(&tv, nullptr);  // beállítja a rendszeridőt

      strftime(buf, sizeof(buf), "Received time %a %y-%m-%d %H:%M", &t);
      // sprintf(buf, "%s %02u-%02u-%02u %02u:%02u", DWS[mytime.dow], mytime.year, mytime.month, mytime.day, mytime.hour, mytime.minute);
      //Serial.println(buf);
      xQueueSend(logQueue, buf, portMAX_DELAY);

      // u8g2.setCursor(0, 48);
      // u8g2.print(buf);	// write something to the internal memory
      // u8g2.sendBuffer();					// transfer internal memory to the display

    }
    vTaskDelay(1);
  }
}



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

  xTaskCreate(blinkTask, "Blink", 2048, NULL, 1, NULL);
  xTaskCreate(loggerTask, "Logger", 2048, NULL, 1, NULL);
  xTaskCreate(dcfTask, "DCF receiver", 2048, NULL, 1, NULL);
  xTaskCreate(scrTask, "Screen refresh", 2048, NULL, 1, NULL);

}


void loop() {

  vTaskDelay(pdMS_TO_TICKS(DISP_PERIOD));
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
