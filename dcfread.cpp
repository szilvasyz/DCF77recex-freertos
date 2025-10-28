#include "common.h"
#include "sysclk.h"


void initDcf() {
  #if (DCF_BLINK != 0)
    receiver.begin(LED_BUILTIN);
  #else
    receiver.begin(PIN_NONE);
  #endif
}


void dcfTask(void* pvParameters) {
  DCFtime rectime;
  struct tm dcf_tm;

  while(true) {
  
    receiver.getTime(&rectime);

    if (rectime.newtime) {
      //struct tm t = {0};

      dcf_tm = makeTm(2000 + rectime.year, rectime.month, rectime.day, rectime.hour, rectime.minute);
      time_t now = utcFromLocal(&dcf_tm, DCF_TZ); // Unix időbélyeg

      lastDCFtime = now;
      sprintf(buf, "DCF sync epoch: %llu", now);
      xQueueSend(logQueue, buf, portMAX_DELAY);

      timeSetUtc(now);
      lastsynced = now;

      strftime(buf, sizeof(buf), "DCF sync time: %a %y-%m-%d %H:%M", gmtime_r(&now, &dcf_tm));

      xQueueSend(logQueue, buf, portMAX_DELAY);

      // u8g2.setCursor(0, 48);
      // u8g2.print(buf);	// write something to the internal memory
      // u8g2.sendBuffer();					// transfer internal memory to the display

    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}
