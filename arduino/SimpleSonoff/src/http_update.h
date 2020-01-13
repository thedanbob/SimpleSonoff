#ifndef SIMPLE_SONOFF_HTTP_UPDATE_H
#define SIMPLE_SONOFF_HTTP_UPDATE_H

#include "../defines.h"
#include "hardware.h"

namespace SimpleSonoff {
  class HTTPUpdate {
    bool _inProgress;

    public:
      HTTPUpdate();
      void setup(SimpleSonoff::Hardware &hardware);
      void checkUpdate();
      bool inProgress();
  };
}

#endif
