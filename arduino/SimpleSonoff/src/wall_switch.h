#ifndef SIMPLE_SONOFF_WALL_SWITCH_H
#define SIMPLE_SONOFF_WALL_SWITCH_H

#include "../defines.h"
#include "hardware.h"

namespace SimpleSonoff {
  class WallSwitch {
    bool _lastState;
    SimpleSonoff::Hardware *_hardware;

    public:
      WallSwitch(SimpleSonoff::Hardware &h);
      void setup();
      void check();
  };
}

#endif
