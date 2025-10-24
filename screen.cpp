#include "common.h"


void initScr() {
  u8g2Mutex = xSemaphoreCreateMutex();

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

  for (;;) {
    vTaskDelayUntil(&lastWakeTime, period);  
  
    heartbeat = !heartbeat;
    time_t t = time(nullptr);

    xSemaphoreTake(u8g2Mutex, portMAX_DELAY);
    u8g2.setCursor(0,0);
    u8g2.print("*O"[heartbeat]);
    u8g2.print(" S"[(t - lastsynced) < SYNC_PERIOD_S]);
    xSemaphoreGive(u8g2Mutex);

    u8g2.sendBuffer();
  }
}
