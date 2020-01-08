#ifndef SIMPLE_SONOFF_HARDWARE_H
#define SIMPLE_SONOFF_HARDWARE_H

#include <Arduino.h>
#include <Ticker.h>
#include "defines.h"

namespace SimpleSonoff {
  class Hardware {
    static const int _ledPin;
    static const int _btnPin[CHANNELS];
    static const int _relayPin[CHANNELS];
    static const bool _rememberState[CHANNELS];

    bool _sendState[CHANNELS];
    unsigned long _btnCount[CHANNELS];
    Ticker _btnTimer[CHANNELS];
    bool _restart;

    void _setupChannel(int ch);
    void _buttonHandler(int ch);

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
      bool restart();
  };
}

#endif
