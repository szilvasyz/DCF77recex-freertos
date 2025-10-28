# DCF77recex-freertos


- config.h - defines of hardware connections, working parameters
- common.h - references to common resources
- blink.h/cpp - blinking "heartbeat" task
- logger.h/cpp - thread-safe logging to serial output
- screen.h/cpp - basic screen handling and refreshing
- dcfread.h/cpp - read local time from DCF77 receiver module
- sysclk.h/cpp - some time/RTC manipulation and synchronization routines
- gpsread.h/cpp - read UTC time from serial connected GPS module


## GPS handler FSM

The diagram shows the GPS autoinitialization FSM. All UART or hardware failures will force the FSM into the ERROR state
(outside of the GPS handler block) where manual intervention is required.

```mermaid
stateDiagram-v2
    state "GPS handler" as GPS_BLOCK {
      direction TB

      [*] --> DISCONNECTED : GPS_FAST_START==0
      [*] --> CONNECTING : GPS_FAST_START==1

      DISCONNECTED : <b>DISCONNECTED</b><br/>prepare for connection</br><small>- delay 10sec (debouncing)</small>
      REINIT : <b>REINIT</b><br/>set GPS parameters<br/><small>- delay 500ms<br/>- send init commands<br/>- delay 500ms</small>
      CONNECTING : <b>CONNECTING</b><br/>wait for valid message<br/><small>- open serial connection</small>
      CONNECTED : <b>CONNECTED</b><br/>processing messages<br/><small>- receiving NMEA message<br/>- processing NMEA message</small>


      DISCONNECTED --> REINIT : USE_GPS_REINIT==1
      DISCONNECTED --> CONNECTING : USE_GPS_REINIT==0

      REINIT --> CONNECTING

      CONNECTING --> CONNECTED : new message
      CONNECTING --> DISCONNECTED : timeout (no message)

      CONNECTED --> DISCONNECTED : timeout (no message)


      %% DISCONNECTED --> ERROR : UART error
      %% REINIT --> ERROR : UART error
      %% CONNECTING --> ERROR : UART error
      %% CONNECTED --> ERROR : UART error
    }

    ERROR : <b>ERROR</b><br/>FSM lands here in case of any failure<br/><small>- manual intervention needed</small>

```
