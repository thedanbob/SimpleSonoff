#ifndef SIMPLE_SONOFF_HARDWARE_H
#define SIMPLE_SONOFF_HARDWARE_H

#include <Ticker.h>
#include "config.h"

namespace SimpleSonoff {
  class Hardware {
    static const int ledPin;
    static const int btnPin[4];
    static const int relayPin[4];
    static const bool rememberState[4];
    static const String stateName[2];

    bool relayState[4];
    bool sendState[4];
    unsigned long btnCount[4];
    Ticker btnTimer[4];
    bool restart;

    void setupChannel(int ch);
    void buttonHandler(int ch);

    public:
      Relay();
      void init();
      void finishInit();
      void blinkLED(int duration, int n);
      bool getLED();
      void setLED(bool on);
      void setRelay(int ch, bool state);
      bool shouldSendState(int ch);
      String checkState(int ch);
      void toggleWallSwitch();
      bool requestRestart();
  };
}

#endif
