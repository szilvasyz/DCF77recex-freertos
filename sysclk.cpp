#include "common.h"
#include <sys/time.h>
#include <string.h>
#include "sysclk.h"


void initSysClk() {
  
  setenv("TZ", "UTC0", 1);
  tzset();

  struct tm timeinfo = makeTm(2020, 1, 1, 0, 0, 0);
  time_t now = mktime(&timeinfo);  // Unix időbélyeg
  struct timeval tv = { now, 0 };
  settimeofday(&tv, nullptr);  // beállítja a rendszeridőt

  last = millis();

}


// --- A kulcsfüggvény ---
time_t utcFromLocal(const struct tm* local, const char* tz) {
  // Elmentjük az aktuális időzónát
  char prevTZ[64] = "";
  char bb[80];

  xSemaphoreTake(tzMutex, portMAX_DELAY);
  const char* old = getenv("TZ");
  if (old) strncpy(prevTZ, old, sizeof(prevTZ));

  // Beállítjuk az ideiglenes időzónát
  setenv("TZ", tz, 1);
  tzset();

  // A megadott helyi időből kiszámítjuk az ennek megfelelő UTC-t
  struct tm tmp = *local;
  time_t utc = mktime(&tmp);  // mktime a TZ-hez viszonyított lokálból UTC-t ad

  sprintf(bb, "%s: %02d:%02d -> %lld", tz, local->tm_hour, local->tm_min, utc);
  xQueueSend(logQueue, bb, portMAX_DELAY);

  // Visszaállítjuk az eredeti TZ-t
  setenv("TZ", prevTZ, 1);
  tzset();
  xSemaphoreGive(tzMutex);

  return utc;
}


time_t timeGetUtc() {
  time_t now;
  time(&now);
  return now;
}


void timeSetUtc(time_t now) {
  struct timeval tv = { now, 0 };
  xSemaphoreTake(tzMutex, portMAX_DELAY);
  settimeofday(&tv, nullptr);
  xSemaphoreGive(tzMutex);
  return;
}


struct tm* timeGetLocal(struct tm* out, const char* targetTZ) {
  time_t now = timeGetUtc();
  return timeToLocal(out, now, targetTZ);
}


struct tm* timeToLocal(struct tm* out, time_t utc, const char* targetTZ) {
  char prevTZ[64] = "";

  xSemaphoreTake(tzMutex, portMAX_DELAY);
  const char* old = getenv("TZ");
  if (old) strncpy(prevTZ, old, sizeof(prevTZ));

  setenv("TZ", targetTZ, 1);
  tzset();

  localtime_r(&utc, out);

  setenv("TZ", prevTZ, 1);
  tzset();
  xSemaphoreGive(tzMutex);

  return out;
}

