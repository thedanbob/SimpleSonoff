#ifndef SIMPLE_SONOFF_WALL_SWITCH_H
#define SIMPLE_SONOFF_WALL_SWITCH_H

#include <memory>
#include "config.h"
#include "hardware.h"

namespace SimpleSonoff {
  class WallSwitch {
    bool lastState;
    std::unique_ptr<SimpleSonoff::Hardware> hardware;

    public:
      WallSwitch(SimpleSonoff::Hardware hardware);
      void setup();
      void check();
  };
}

#endif
