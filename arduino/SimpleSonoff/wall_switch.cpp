#include "wall_switch.h"

namespace SimpleSonoff {
  WallSwitch::WallSwitch(SimpleSonoff::Hardware* h) :
    lastState(true),
    hardware(h)
  {}

  void WallSwitch::setup() {
    pinMode(OPT_PIN, INPUT_PULLUP);
  }

  void WallSwitch::check() {
    bool state = digitalRead(OPT_PIN);
    if (state != this->lastState) {
      hardware->setRelay(0, !hardware->getRelay(0));
    }
    this->lastState = state;
  }
}
