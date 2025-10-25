# DCF77recex-freertos


config.h - defines of hardware connections, working parameters
common.h - references to common resources
blink.h/cpp - blinking "heartbeat" task
logger.h/cpp - thread-safe logging to serial output
screen.h/cpp - basic screen handling and refreshing
dcfread.h/cpp - read local time from DCF77 receiver module
sysclk.h/cpp - some time/RTC manipulation and synchronization routines
gpsread.h/cpp - read UTC time from serial connected GPS module (future)



```mermaid
stateDiagram-v2
    direction TB

    [*] --> DISCONNECTED

    DISCONNECTED : DISCONNECTED<br/><small>várakozás / debounce</small><br/>• Indít 10 s timert<br/>• Timer lejár után döntés CONNECTING vagy REINIT felé
    DISCONNECTED --> CONNECTING : 10 s letelt
    DISCONNECTED --> REINIT : 10 s letelt és USE_GPS_REINIT definiálva
    DISCONNECTED --> ERROR : hardware vagy UART hiba

    REINIT : REINIT<br/><small>GPS újrainicializálás</small><br/>• GPS reset pin LOW<br/>• Várakozás 500 ms<br/>• INIT parancsok küldése<br/>• UART buffer flush<br/>• Kilépés CONNECTING-be
    REINIT --> CONNECTING : újrainicializálás sikeres
    REINIT --> ERROR : hardware vagy UART hiba

    CONNECTING : CONNECTING<br/><small>adatvárás</small><br/>• Soros kapcsolat nyitása<br/>• NMEA üzenetek figyelése
    CONNECTING --> CONNECTED : gps_message_ok()
    CONNECTING --> DISCONNECTED : timeout (nem jön értelmezhető adat)
    CONNECTING --> ERROR : hardware vagy UART hiba

    CONNECTED : CONNECTED<br/><small>stabil adatfolyam</small><br/>• GPS érvényes adatokat küld
    CONNECTED --> DISCONNECTED : timeout (adatvesztés)
    CONNECTED --> ERROR : hardware vagy UART hiba

    ERROR : ERROR<br/><small>végállapot</small><br/>• Manuális beavatkozás szükséges
