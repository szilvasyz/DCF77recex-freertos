// ================= dcf77_rtos.cpp =================
#include "dcf77_rtos.h"

#ifndef DCF_PIN
#define DCF_PIN 27
#endif

// --- bit helpers ---------------------------------------------------
inline bool DCF77FreeRTOS::dbit(uint64_t v,int pos){ return (v>>pos)&1ULL; }
inline uint64_t DCF77FreeRTOS::dslice(uint64_t v,int lsb,int msb){
  uint64_t mask=((1ULL<<(msb-lsb+1))-1ULL)<<lsb;
  return (v&mask)>>lsb;
}
inline bool DCF77FreeRTOS::parityEven(uint64_t v,int from,int to){
  return (__builtin_popcountll(dslice(v,from,to))&1)==0;
}
inline int DCF77FreeRTOS::bcd(uint64_t v,int lsb,int msb){
  return static_cast<int>(dslice(v,lsb,msb));
}
// -------------------------------------------------------------------

static const int PW[]={35,135,250,750,850,950,1750,1850,1950,0};
static const int PN[]={0,1,2,0,3,4,0,5,6,0};

DCF77FreeRTOS::DCF77FreeRTOS(int pin,int dbg)
: _pin(pin),_dbg(dbg),_q(nullptr),_ev(nullptr),
  _mux(portMUX_INITIALIZER_UNLOCKED),recbits(0),rbnum(0){}

void DCF77FreeRTOS::begin(int ledPin){
  _ledPin = ledPin;
  _q=xQueueCreate(64,sizeof(Pulse));
  _ev=xEventGroupCreate();
  pinMode(_pin,INPUT);
  if (_ledPin >= 0) pinMode(_ledPin, OUTPUT);
  gpio_set_intr_type((gpio_num_t)_pin,GPIO_INTR_ANYEDGE);
  gpio_install_isr_service(0);
  gpio_isr_handler_add((gpio_num_t)_pin,&DCF77FreeRTOS::isrThunk,this);
  xTaskCreatePinnedToCore(&DCF77FreeRTOS::taskThunk,"dcf77",
      DCF_TASK_STACK,this,DCF_TASK_PRIO,nullptr,1);
}

bool DCF77FreeRTOS::getTime(DCFtime* out){
  if (!out) return false;
  taskENTER_CRITICAL(&_mux);
  bool nt = _rectime.newtime;
  *out = _rectime;
  _rectime.newtime = 0;
  taskEXIT_CRITICAL(&_mux);
  return nt;
}

EventGroupHandle_t DCF77FreeRTOS::events()const{ return _ev; }

void IRAM_ATTR DCF77FreeRTOS::isrThunk(void* arg){
  reinterpret_cast<DCF77FreeRTOS*>(arg)->onEdgeISR();
}

void IRAM_ATTR DCF77FreeRTOS::onEdgeISR(){
  static uint32_t last_ms=0;
  uint32_t now_ms=millis();
  uint32_t diff=last_ms?(now_ms-last_ms):0;
  last_ms=now_ms;
  uint8_t lvl=(uint8_t)gpio_get_level((gpio_num_t)_pin);

  if (_ledPin >= 0) gpio_set_level((gpio_num_t)_ledPin, lvl);  // debug LED villogtatás

  Pulse p{diff,lvl};
  BaseType_t hpw=pdFALSE;
  if(_q)xQueueSendFromISR(_q,&p,&hpw);
  if(hpw)portYIELD_FROM_ISR();
}

void DCF77FreeRTOS::taskThunk(void* arg){
  reinterpret_cast<DCF77FreeRTOS*>(arg)->runTask();
}

void DCF77FreeRTOS::runTask(){
  Pulse p;
  while(true){
    if(xQueueReceive(_q,&p,portMAX_DELAY)==pdTRUE)consumePulse_(p);
  }
}

DCF77FreeRTOS::IMP DCF77FreeRTOS::classify_(uint32_t dt_ms){
  int i=0; while(PW[i]!=0 && dt_ms>(uint32_t)PW[i]) i++;
  switch(PN[i]){
    case 1: return SPC0; case 2: return SPC1; case 3: return MRK1;
    case 4: return MRK0; case 5: return LST1; case 6: return LST0;
    default:return INVD;
  }
}

void DCF77FreeRTOS::consumePulse_(const Pulse& p){
  IMP sym=classify_(p.dt_ms);
  if(_dbg)Serial.printf("%u ms => %d\n",(unsigned)p.dt_ms,(int)sym);
  switch(sym){
    case SPC0:
      recbits |= (0ULL<<rbnum++);
      break;
    case SPC1:
      recbits |= ((uint64_t)1<<rbnum++);
      break;
    case LST1: case LST0:
      if(rbnum>=59)decodeFrame_(); else xEventGroupSetBits(_ev,DCF_EVENT_ERROR);
      recbits=0; rbnum=0; break;
    default: break;
  }
}

void DCF77FreeRTOS::decodeFrame_(){
  if(_dbg) {
    Serial.printf("[DCF] Frame (rbnum=%d): 0x%016llX\n", rbnum, recbits);
    for (int i = 58; i >= 0; i--) {
        Serial.print((recbits >> i) & 1 ? '1' : '0');
        if (i % 4 == 0) Serial.print(' ');  // nibble-határok
    }
    Serial.println();
    Serial.printf("bit20=%d bit21=%d bit58=%d\n",
              dbit(recbits,20),
              dbit(recbits,21),
              dbit(recbits,58));
  }

  if(!dbit(recbits,20)){ xEventGroupSetBits(_ev,DCF_EVENT_ERROR); return; }

  bool p1=parityEven(recbits,21,27)==!dbit(recbits,28);
  bool p2=parityEven(recbits,29,34)==!dbit(recbits,35);
  bool p3=parityEven(recbits,36,57)==!dbit(recbits,58);
  if(!(p1&&p2&&p3)){ xEventGroupSetBits(_ev,DCF_EVENT_ERROR); return; }

  DCFtime t;
  t.minute=bcd(recbits,21,24) + 10 * bcd(recbits,25,27);
  t.hour  =bcd(recbits,29,32) + 10 * bcd(recbits,33,34);
  t.day   =bcd(recbits,36,39) + 10 * bcd(recbits,40,41);
  t.dow   =bcd(recbits,42,44);
  t.month =bcd(recbits,45,48) + 10 * dbit(recbits,49);
  t.year  =bcd(recbits,50,53) + 10 * bcd(recbits,54,57);
  t.dst   =dbit(recbits,17);
  t.tstamp=millis();
  t.newtime=1;

  if(t.minute>59||t.hour>23||t.day<1||t.day>31||t.month<1||t.month>12){
    xEventGroupSetBits(_ev,DCF_EVENT_ERROR); return;
  }

  taskENTER_CRITICAL(&_mux);
  _rectime=t;
  taskEXIT_CRITICAL(&_mux);
  xEventGroupSetBits(_ev,DCF_EVENT_NEW_MINUTE|DCF_EVENT_SYNCED);
}

// ================= example =================
/*
#include \"dcf77_rtos.h\"
DCF77FreeRTOS dcf;
void setup(){
  Serial.begin(115200);
  dcf.begin();
}
void loop(){
  DCFtime t;
  if(dcf.getTime(t)){
    Serial.printf(\"%04d-%02d-%02d %02d:%02d (dow=%d)\\n\",
      t.year,t.month,t.day,t.hour,t.minute,t.dow);
  }
  vTaskDelay(pdMS_TO_TICKS(50));
}
*/
