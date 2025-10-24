#include "common.h"


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
