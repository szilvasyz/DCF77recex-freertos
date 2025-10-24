#include "common.h"


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
