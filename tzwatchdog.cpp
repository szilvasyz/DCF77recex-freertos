#include "common.h"
#include <time.h>

#define TZ_WATCHDOG_PERIOD_MS   (60 * 1000)    // 1 perc
#define TZ_EXPECTED_OFFSET_SEC  0              // UTC0, ha UTC az alap

void tzWatchdogTask(void* pvParameters)
{
    char buf[80];
    for (;;) {
        time_t now = time(nullptr);
        struct tm loc, utc;

        localtime_r(&now, &loc);
        gmtime_r(&now, &utc);

        time_t t_loc = mktime(&loc);
        time_t t_utc = mktime(&utc);

        long offset = difftime(t_loc, t_utc);

        sprintf(buf, "%9d", offset);
        xSemaphoreTake(u8g2Mutex, portMAX_DELAY);
        u8g2.setCursor(64, 0);
        u8g2.print(buf);	// write something to the internal memory
        xSemaphoreGive(u8g2Mutex);

        snprintf(buf, sizeof(buf), "TZ watchdog: Δ=%ld sec, TZ: %s", offset, getenv("TZ"));
        xQueueSend(logQueue, buf, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(TZ_WATCHDOG_PERIOD_MS));
    }
}
