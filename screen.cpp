#include "common.h"
#include "sysclk.h"


void initScr() {

  xSemaphoreTake(u8g2Mutex, portMAX_DELAY);

  u8g2.begin();
  //u8g2.setFont(u8g2_font_rosencrantz_nbp_tf);	// choose a suitable font
  //u8g2.setFont(u8g2_font_profont11_mf);	// choose a suitable font
  u8g2.setFont(u8g2_font_7x14_mf);	// choose a suitable font
   
  u8g2.setFontPosTop();
  u8g2.setFontMode(0);

  u8g2.setCursor(0, 48);
  u8g2.print("Starting...234567890");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display

  xSemaphoreGive(u8g2Mutex);


}


void scrTask(void* pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(DISP_PERIOD_MS);
  struct tm timeinfo;

  for (;;) {
    vTaskDelayUntil(&lastWakeTime, period);  
  
    heartbeat = !heartbeat;

    xSemaphoreTake(u8g2Mutex, portMAX_DELAY);
    u8g2.setCursor(0,0);
    u8g2.print("*O"[heartbeat]);
    u8g2.print(" S"[(timeGetUtc() - lastsynced) < SYNC_PERIOD_S]);

    //time_t now = time(nullptr);
    //localtime_r(&now, &timeinfo);
    //getLocalTime(&timeinfo, 20);  // lekérdezzük az aktuális időt (fut az ESP32 RTC-je)
    //strftime(buf, sizeof(buf), "%a %y-%m-%d %H:%M", &timeinfo);

    strftime(buf, sizeof(buf), "%a %y-%m-%d %H:%M", timeGetLocal(&timeinfo, DISP_TZ));
    u8g2.setCursor(0, 16);
    u8g2.print(buf);	// write something to the internal memory

    strftime(buf, sizeof(buf), "%a %y-%m-%d %H:%M", timeToLocal(&timeinfo, lastDCFtime, DISP_TZ));
    u8g2.setCursor(0, 32);
    u8g2.print(buf);	// write something to the internal memory

    strftime(buf, sizeof(buf), "%a %y-%m-%d %H:%M", timeToLocal(&timeinfo, lastGPStime, DISP_TZ));
    u8g2.setCursor(0, 48);
    u8g2.print(buf);	// write something to the internal memory

    xSemaphoreGive(u8g2Mutex);

    u8g2.sendBuffer();
  }
}
