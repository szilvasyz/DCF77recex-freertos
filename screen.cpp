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
  u8g2.print("Starting...");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display

  xSemaphoreGive(u8g2Mutex);


}


void scrTask(void* pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(DISP_PERIOD_MS);
  struct tm timeinfo;
  time_t now;

  for (;;) {
    vTaskDelayUntil(&lastWakeTime, period);  
  
    heartbeat = !heartbeat;

    xSemaphoreTake(u8g2Mutex, portMAX_DELAY);
    u8g2.setCursor(0,0);
    u8g2.print("*O"[heartbeat]);
    now = getUtcTime();
    u8g2.print(" S"[isTimeSyncAging() ? isTimeSynced() ? heartbeat : 0 : 1]);
    u8g2.print(" D"[now - lastDCFtime < SYNC_INTERVAL_SEC]);
    u8g2.print(" G"[now - lastGPStime < SYNC_INTERVAL_SEC]);
    

    localFromUtc(now, DISPrule, &timeinfo);
    u8g2.setCursor(0, 16);
    strftime(buf, sizeof(buf), "%a %y%m%d %H:%M ", &timeinfo);
    u8g2.print(buf);	// write something to the internal memory
    u8g2.print(timeinfo.tm_isdst ? '*' : ' ');

    localFromUtc(lastDCFtime, DISPrule, &timeinfo);
    u8g2.setCursor(0, 32);
    strftime(buf, sizeof(buf), "%a %y%m%d %H:%M ", &timeinfo);
    u8g2.print(buf);	// write something to the internal memory
    u8g2.print(timeinfo.tm_isdst ? '*' : ' ');

    localFromUtc(lastGPStime, DISPrule, &timeinfo);
    u8g2.setCursor(0, 48);
    strftime(buf, sizeof(buf), "%a %y%m%d %H:%M ", &timeinfo);
    u8g2.print(buf);	// write something to the internal memory
    u8g2.print(timeinfo.tm_isdst ? '*' : ' ');

    xSemaphoreGive(u8g2Mutex);

    u8g2.sendBuffer();
  }
}
