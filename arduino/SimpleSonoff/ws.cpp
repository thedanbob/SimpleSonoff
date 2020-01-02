#include "config.h"
#include "ws.h"

namespace SimpleSonoff {
  WS::WS(SimpleSonoff::Hardware hardware) {
    this->lastState = true;
    this->hardware.reset(&hardware);
  }

  void WS::setup() {
    pinMode(OPT_PIN, INPUT_PULLUP);
  }

  void WS::check() {
    bool state = digitalRead(OPT_PIN);
    if (state != this->lastState) {
      hardware->toggleWallSwitch();
    }
    this->lastState = state;
  }
}
