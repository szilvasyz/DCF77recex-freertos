#include "common.h"


void loggerTask(void* pvParameters) {
  char msg[MAX_LOG_MSG_LEN];
  for (;;) {
    if (xQueueReceive(logQueue, msg, portMAX_DELAY)) {
      Serial.println(msg);
    }
  }
}
