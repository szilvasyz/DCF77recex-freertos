#include "common.h"

#include "sysclk.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ----------------------------------------------------------
// Helper: nth / last weekday of month
// ----------------------------------------------------------

static inline void lastWeekdayOfMonth(struct tm* out,
                                      int year, int month, int wday,
                                      int hour, int min, int sec)
{
    struct tm t = makeTm(year - 1900, month, 0, hour, min, sec);
    mktime(&t);
    while (t.tm_wday != wday) { t.tm_mday--; mktime(&t); }
    *out = t;
}

static inline void nthWeekdayOfMonth(struct tm* out,
                                     int year, int month, int n, int wday,
                                     int hour, int min, int sec)
{
    struct tm t = makeTm(year - 1900, month - 1, 1, hour, min, sec);
    mktime(&t);
    int delta = (wday - t.tm_wday + 7) % 7;
    t.tm_mday += delta + 7*(n-1);
    mktime(&t);
    *out = t;
}

// ----------------------------------------------------------
// POSIX TZ string parser (e.g. CET-1CEST,M3.5.0/2,M10.5.0/3)
// ----------------------------------------------------------

bool parseTzString(const char* tz, TzRule& rule)
{
    memset(&rule, 0, sizeof(rule));
    rule.baseOffset = 0;
    rule.dstOffset  = 0;

    const char* p = tz;
    int i = 0;

    // --- STD name ---
    while (*p && ((*p >= 'A' && *p <= 'Z'))) {
        if (i < 7) rule.stdName[i++] = *p;
        p++;
    }
    rule.stdName[i] = '\0';

    // --- Base offset ---
    int sign = -1;
    if (*p == '+') { sign = -1; p++; }   // POSIX: CET-1 -> UTC+1
    else if (*p == '-') { sign = 1; p++; }

    int h=0, m=0, s=0;
    sscanf(p, "%d:%d:%d", &h, &m, &s);
    rule.baseOffset = sign * (h*3600 + m*60 + s);

    // Skip numeric part
    while (*p && (isdigit(*p) || *p == ':')) p++;

    // --- DST name (optional) ---
    i = 0;
    if (*p && *p != ',' && (*p >= 'A' && *p <= 'Z')) {
        while (*p && (*p >= 'A' && *p <= 'Z')) {
            if (i < 7) rule.dstName[i++] = *p;
            p++;
        }
    }
    rule.dstName[i] = '\0';
    if (!rule.dstName[0]) strcpy(rule.dstName, rule.stdName);

    // --- DST offset (optional) ---
    sign = -1;
    if (*p == '+') { sign = -1; p++; }
    else if (*p == '-') { sign = 1; p++; }

    bool hasDSTrule = strchr(tz, ',') != nullptr;

    if (isdigit(*p)) {
        h = m = s = 0;
        sscanf(p, "%d:%d:%d", &h, &m, &s);
        rule.dstOffset = sign * (h*3600 + m*60 + s);
    } else if (rule.dstName[0] && hasDSTrule) {
        // only add +1h if DST rule or DST name explicitly present
        rule.dstOffset = rule.baseOffset + 3600;
    } else {
        // no DST
        rule.dstOffset = rule.baseOffset;
    }

    // --- DST rules ---
    const char* firstComma = strchr(tz, ',');
    const char* secondComma = firstComma ? strchr(firstComma + 1, ',') : nullptr;
    if (firstComma) {
        sscanf(firstComma, ",M%d.%d.%d/%d:%d:%d",
               &rule.startMonth, &rule.startWeek, &rule.startWday,
               &rule.startHour, &rule.startMin, &rule.startSec);
    }
    if (secondComma) {
        sscanf(secondComma, ",M%d.%d.%d/%d:%d:%d",
               &rule.endMonth, &rule.endWeek, &rule.endWday,
               &rule.endHour, &rule.endMin, &rule.endSec);
    }

    // defaults if rule missing
    if (!rule.startMonth && hasDSTrule) {
        rule.startMonth=3; rule.startWeek=5; rule.startWday=0; rule.startHour=1;
    }
    if (!rule.endMonth && hasDSTrule) {
        rule.endMonth=10; rule.endWeek=5; rule.endWday=0; rule.endHour=1;
    }

    return true;
}

// ----------------------------------------------------------
// Compute current offset (DST-aware)
// ----------------------------------------------------------

long tzOffsetFor(time_t utc, const TzRule& rule, bool* isDst)
{
    struct tm tm_utc;
    gmtime_r(&utc, &tm_utc);
    int year = tm_utc.tm_year + 1900;

    struct tm startLocal, endLocal;
    if (rule.startWeek == 5)
        lastWeekdayOfMonth(&startLocal, year, rule.startMonth, rule.startWday,
                           rule.startHour, rule.startMin, rule.startSec);
    else
        nthWeekdayOfMonth(&startLocal, year, rule.startMonth, rule.startWeek,
                          rule.startWday, rule.startHour, rule.startMin, rule.startSec);

    if (rule.endWeek == 5)
        lastWeekdayOfMonth(&endLocal, year, rule.endMonth, rule.endWday,
                           rule.endHour, rule.endMin, rule.endSec);
    else
        nthWeekdayOfMonth(&endLocal, year, rule.endMonth, rule.endWeek,
                          rule.endWday, rule.endHour, rule.endMin, rule.endSec);

    // Convert local times to UTC reference
    time_t start_t_local = mktime(&startLocal);
    time_t end_t_local   = mktime(&endLocal);
    time_t start_t = start_t_local - rule.baseOffset;  // local->UTC
    time_t end_t   = end_t_local   - rule.dstOffset;   // DST->UTC

    bool dstActive;
    
    if (start_t < end_t) {
        // normál (északi félteke) logika
        dstActive = (utc >= start_t && utc < end_t);
    } else {
        // déli félteke: átnyúlik az évhatáron
        dstActive = (utc >= start_t || utc < end_t);
    }

    if (isDst) *isDst = dstActive;

    return dstActive ? rule.dstOffset : rule.baseOffset;
}

// ----------------------------------------------------------
// Local → UTC
// ----------------------------------------------------------

time_t utcFromLocal(const struct tm* local, const TzRule& rule)
{
    struct tm tmp = *local;
    time_t local_epoch = mktime(&tmp);
    long offset = tzOffsetFor(local_epoch - rule.baseOffset, rule);
    return local_epoch - offset;
}

// ----------------------------------------------------------
// UTC → Local
// ----------------------------------------------------------

void localFromUtc(time_t utc, const TzRule& rule, struct tm* out)
{
    bool isDst = false;
    long offset = tzOffsetFor(utc, rule, &isDst);

    time_t local_epoch = utc + offset;
    gmtime_r(&local_epoch, out);
    out->tm_isdst = isDst ? 1 : 0;
}


bool isTimeSynced()
{
    time_t now = getUtcTime();
    return (now - lastsynced) < STALE_TIMEOUT_SEC;
}


bool isTimeSyncAging()
{
    time_t now = getUtcTime();
    return (now - lastsynced) > AGING_TIMEOUT_SEC;
}


bool tryTimeSync(const char* source, time_t utcNow)
{
    time_t now = getUtcTime();
    time_t delta = now - lastsynced;

    // túl gyakori szinkronizálás tiltása
    if (delta < SYNC_INTERVAL_SEC) {
        return false;   // most nem frissítünk
    }

    // csak érvényes, nem nulla időbélyeget fogadjunk el
    if (utcNow < 1000000000) return false;

    // beállítjuk a rendszeridőt
    setUtcTime(utcNow);
    lastsynced = utcNow;

    char msg[64];
    snprintf(msg, sizeof(msg), "Time synced from %s: %ld", source, utcNow);
    xQueueSend(logQueue, msg, 0);

    return true;
}






