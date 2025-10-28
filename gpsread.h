#pragma once


void initGps();
bool parseRMC(const char* sentence, struct tm* out);
void gpsCopyTask(void* pvParameters);
void gpsTask(void* pvParameters);
