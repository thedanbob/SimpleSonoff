#ifndef SIMPLE_SONOFF_WS_H
#define SIMPLE_SONOFF_WS_H

#include <memory>
#include "config.h"
#include "hardware.h"

namespace SimpleSonoff {
  class WS {
    bool lastState;
    std::unique_ptr<SimpleSonoff::Hardware> hardware;

    public:
      WS(SimpleSonoff::Hardware hardware);
      void setup();
      void check();
  };
}

#endif
