#include "config.h"
#include "hardware.h"
#include "wall_switch.h"

namespace SimpleSonoff {
  WallSwitch::WallSwitch(SimpleSonoff::Hardware hardware) {
    this->lastState = true;
    this->hardware.reset(&hardware);
  }

  void WallSwitch::setup() {
    pinMode(OPT_PIN, INPUT_PULLUP);
  }

  void WallSwitch::check() {
    bool state = digitalRead(OPT_PIN);
    if (state != this->lastState) {
      this->hardware->toggleWallSwitch();
    }
    this->lastState = state;
  }
}
