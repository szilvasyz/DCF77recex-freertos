#pragma once

#include <Arduino.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

// ==========================================================
//   Time Zone Conversion Library for ESP32 / FreeRTOS
//   Supports POSIX-style TZ strings like:
//   "CET-1CEST,M3.5.0/2,M10.5.0/3"
// ==========================================================

struct TzRule {
    char stdName[8];     // Standard zone name (e.g. CET)
    char dstName[8];     // DST zone name (e.g. CEST)
    int baseOffset;      // Standard offset in seconds (e.g. +3600)
    int dstOffset;       // DST offset in seconds (e.g. +7200)
    int startMonth;      // DST start month (1–12)
    int startWeek;       // DST start week (1–5, 5=last)
    int startWday;       // DST start weekday (0=Sun)
    int startHour;
    int startMin;
    int startSec;
    int endMonth;        // DST end month
    int endWeek;
    int endWday;
    int endHour;
    int endMin;
    int endSec;
};

// --- Interface --------------------------------------------------------------

static inline void setUtcTime(time_t utc) { struct timeval tv={utc, 0}; settimeofday(&tv, nullptr); }
static inline time_t getUtcTime() { return time(nullptr); }
bool parseTzString(const char* tz, TzRule& rule);
long tzOffsetFor(time_t utc, const TzRule& rule, bool* isDst = nullptr);
time_t utcFromLocal(const struct tm* local, const TzRule& rule);
void localFromUtc(time_t utc, const TzRule& rule, struct tm* out);

bool isTimeSynced();
bool isTimeSyncAging();
bool tryTimeSync(const char* source, time_t utcNow);



