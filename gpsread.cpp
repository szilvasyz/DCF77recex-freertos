#include "common.h"
#include "sysclk.h"


void initGps() {
  GPS.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}


void gpsCopyTask(void* pvParameters) {
  char line[120];
  size_t idx = 0;

  for (;;) {
    while (GPS.available()) {
      char c = GPS.read();
      if (c == '\n') {
        line[idx] = '\0';
        idx = 0;
        xQueueSend(logQueue, line, pdMS_TO_TICKS(10));
      } else if (idx < sizeof(line)-1) {
        line[idx++] = c;
      }
    }
  
    vTaskDelay(pdMS_TO_TICKS(10));
  }

}


bool parseRMC(const char* sentence, struct tm* out) {
  if (strncmp(sentence, "$GPRMC", 6) && strncmp(sentence, "$GNRMC", 6))
    return false;

  // készíts másolatot, mert belenyúlunk a stringbe
  char copy[100];
  strncpy(copy, sentence, sizeof(copy) - 1);
  copy[sizeof(copy) - 1] = '\0';

  const char* fields[16] = {nullptr};
  size_t count = 0;
  char* p = copy;

  // feldarabolás saját kézzel, nem strtok-kal
  while (p && count < 16) {
    fields[count++] = p;
    char* comma = strchr(p, ',');
    if (!comma) break;
    *comma = '\0';
    p = comma + 1;
  }

  if (count < 10) return false;  // nincs dátummező

  const char* utcField  = fields[1];
  const char* valid     = fields[2];
  const char* dateField = fields[9];

  if (!utcField || !valid || *valid != 'A' || !dateField)
    return false;

  if (strlen(utcField) < 6 || strlen(dateField) < 6)
    return false;

  int hour = (utcField[0]-'0')*10 + (utcField[1]-'0');
  int min  = (utcField[2]-'0')*10 + (utcField[3]-'0');
  int sec  = (utcField[4]-'0')*10 + (utcField[5]-'0');

  int day   = (dateField[0]-'0')*10 + (dateField[1]-'0');
  int month = (dateField[2]-'0')*10 + (dateField[3]-'0');
  int year  = (dateField[4]-'0')*10 + (dateField[5]-'0');

  *out = makeTm(2000 + year, month, day, hour, min, sec);

  return true;
}


// --- Fő GPS task ---
void gpsTask(void* pvParameters) {
  char line[120];
  size_t idx = 0;
  struct tm gps_tm;

  for (;;) {
    while (GPS.available()) {
      char c = GPS.read();
      if (c == '\n') {
        line[idx] = '\0';
        idx = 0;

        if (parseRMC(line, &gps_tm)) {

          if (gps_tm.tm_sec == 0) {
            time_t utc = utcFromLocal(&gps_tm, GPSrule);
            lastGPStime = utc;

            tryTimeSync("GPS", utc);

            char msg[64];
            strftime(msg, sizeof(msg), "GPS sync: %a %y-%m-%d %H:%M", gmtime_r(&utc, &gps_tm));
            xQueueSend(logQueue, msg, pdMS_TO_TICKS(100));
            snprintf(msg, sizeof(msg), "lastGPStime: %llu", lastGPStime);
            xQueueSend(logQueue, msg, pdMS_TO_TICKS(100));
          }
        }
      } else if (idx < sizeof(line)-1) {
        line[idx++] = c;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
