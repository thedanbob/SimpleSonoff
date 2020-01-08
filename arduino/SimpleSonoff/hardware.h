#ifndef SIMPLE_SONOFF_HARDWARE_H
#define SIMPLE_SONOFF_HARDWARE_H

#include <Arduino.h>
#include <Ticker.h>
#include "defines.h"

namespace SimpleSonoff {
  class Hardware {
    static const int ledPin;
    static const int btnPin[CHANNELS];
    static const int relayPin[CHANNELS];
    static const bool rememberState[CHANNELS];

    bool sendState[CHANNELS];
    unsigned long btnCount[CHANNELS];
    Ticker btnTimer[CHANNELS];
    bool restart;

    void setupChannel(int ch);
    void buttonHandler(int ch);

    public:
      Hardware();
      void setup();
      void postSetup();
      void blinkLED(int duration, int n);
      bool getLED();
      void setLED(bool on);
      bool getRelay(int ch);
      void setRelay(int ch, bool state);
      bool getSendState(int ch);
      void setSendState(int ch);
      bool checkState(int ch);
      bool requestRestart();
  };
}

#endif
