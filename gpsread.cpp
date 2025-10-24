#include "common.h"


void initGps() {
  GPS.begin(GPS_BAUD, SERIAL_8N1); // baud megegyezik a moduléval
}


void gpsTask(void* pvParameters) {

  while(true) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
