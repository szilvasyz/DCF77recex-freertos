#include "common.h"


void initSysClk() {  struct tm t = {0};

  // példa: 2020. január 1. 00:00:00
  t.tm_year = 2010 - 1900;  // év - 1900
  t.tm_mon  = 1 - 1;        // hónap (0–11)
  t.tm_mday = 1;            // nap
  t.tm_hour = 0;
  t.tm_min  = 0;
  t.tm_sec  = 0;

  time_t now = mktime(&t);  // Unix időbélyeg
  struct timeval tv = { now, 0 };
  settimeofday(&tv, nullptr);  // beállítja a rendszeridőt

   last = millis();

}
