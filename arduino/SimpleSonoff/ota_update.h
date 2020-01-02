#ifndef SIMPLE_SONOFF_OTA_UPDATE_H
#define SIMPLE_SONOFF_OTA_UPDATE_H

#include "config.h"
#include "hardware.h"

namespace SimpleSonoff {
  class OTAUpdate {
    bool update;
    bool restart;

    public:
      OTAUpdate();
      void setup(char uid[8], SimpleSonoff::Hardware hardware);
      void handle();
      bool doUpdate();
      bool requestRestart();
  };
}

#endif
