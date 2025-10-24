#include "common.h"


void initLogger() { 
  logQueue = xQueueCreate(MAX_LOG_QUE_LEN, MAX_LOG_MSG_LEN);
  Serial.begin(LOG_SPEED);
}


void loggerTask(void* pvParameters) {
  char msg[MAX_LOG_MSG_LEN];
  for (;;) {
    if (xQueueReceive(logQueue, msg, portMAX_DELAY)) {
      Serial.println(msg);
    }
  }
}
