#include "wall_switch.h"

namespace SimpleSonoff {
  WallSwitch::WallSwitch(SimpleSonoff::Hardware &h) :
    _lastState(true),
    _hardware(&h)
  {}

  void WallSwitch::setup() {
    pinMode(OPT_PIN, INPUT_PULLUP);
  }

  void WallSwitch::check() {
    bool state = digitalRead(OPT_PIN);
    if (state != _lastState) {
      _hardware->setRelay(0, !_hardware->getRelay(0));
    }
    _lastState = state;
  }
}
