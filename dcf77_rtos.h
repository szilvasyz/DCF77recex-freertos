// ================= dcf77_rtos.h =================
#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <driver/gpio.h>

struct DCFtime {
  int hour=0, minute=0, year=0, month=0, day=0, dow=0;
  int dst=0;
  unsigned long tstamp=0;
  uint8_t newtime=0;
};

constexpr EventBits_t DCF_EVENT_NEW_MINUTE = (1<<0);
constexpr EventBits_t DCF_EVENT_SYNCED     = (1<<1);
constexpr EventBits_t DCF_EVENT_ERROR      = (1<<2);

#define DCF_TASK_STACK 4096
#define DCF_TASK_PRIO 3

class DCF77FreeRTOS {
public:
  DCF77FreeRTOS(int pin=27,int dbg=0);
  void begin(int ledPin = -1);   // új szignatúra
  bool getTime(DCFtime* out);    // pointeres visszaadás
  EventGroupHandle_t events() const;

private:
  struct Pulse { uint32_t dt_ms; uint8_t level; };
  static void IRAM_ATTR isrThunk(void* arg);
  void IRAM_ATTR onEdgeISR();
  static void taskThunk(void* arg);
  void runTask();

  enum IMP { INVD, SPC0, SPC1, MRK1, MRK0, LST1, LST0 };
  IMP classify_(uint32_t dt_ms);
  void consumePulse_(const Pulse& p);
  void decodeFrame_();

  // helpers (implemented inline in cpp)
  static inline bool  dbit(uint64_t v,int pos);
  static inline uint64_t dslice(uint64_t v,int lsb,int msb);
  static inline bool  parityEven(uint64_t v,int from,int to);
  static inline int   bcd(uint64_t v,int lsb,int msb);

  int _pin,_dbg,_ledPin;
  QueueHandle_t _q;
  EventGroupHandle_t _ev;
  portMUX_TYPE _mux;
  uint64_t recbits;
  int rbnum;
  DCFtime _rectime;
};
