#include "common.h"


void blinkTask(void* pvParameters) {
  pinMode(LED_BUILTIN, OUTPUT);
  for (;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1000));
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

