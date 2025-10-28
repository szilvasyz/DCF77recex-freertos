#pragma once


void initSysClk();
time_t utcFromLocal(const struct tm* local, const char* tz);
time_t timeGetUtc();
void timeSetUtc(time_t now);
struct tm* timeGetLocal(struct tm* out, const char* targetTZ);
struct tm* timeToLocal(struct tm* out, time_t utc, const char* targetTZ);
