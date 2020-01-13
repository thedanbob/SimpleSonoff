#ifndef SIMPLE_SONOFF_OTA_UPDATE_H
#define SIMPLE_SONOFF_OTA_UPDATE_H

#include "../defines.h"
#include "hardware.h"

namespace SimpleSonoff {
  class OTAUpdate {
    bool _inProgress;

    public:
      OTAUpdate();
      void setup(char uid[16], SimpleSonoff::Hardware &hardware);
      void handle();
      bool inProgress();
  };
}

#endif
