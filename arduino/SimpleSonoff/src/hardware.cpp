#include <EEPROM.h>
#include "hardware.h"

namespace SimpleSonoff {
  const int Hardware::_ledPin = LED_PIN;
  #ifdef MULTI
  const int Hardware::_btnPin[] = {BTN_PIN_1, BTN_PIN_2, BTN_PIN_3, BTN_PIN_4};
  const int Hardware::_relayPin[] = {RELAY_PIN_1, RELAY_PIN_2, RELAY_PIN_3, RELAY_PIN_4};
  const bool Hardware::_rememberState[] = {REMEMBER_RELAY_STATE_1, REMEMBER_RELAY_STATE_2, REMEMBER_RELAY_STATE_3, REMEMBER_RELAY_STATE_4};
  #else
  const int Hardware::_btnPin[] = {BTN_PIN_1};
  const int Hardware::_relayPin[] = {RELAY_PIN_1};
  const bool Hardware::_rememberState[] = {REMEMBER_RELAY_STATE_1};
  #endif

  Hardware::Hardware() :
    _restart(false),
    _sendState({false}),
    _btnCount({0})
  {}

  void Hardware::setup() {
    EEPROM.begin(4);
    pinMode(_ledPin, OUTPUT);
    setLED(false);

    _setupChannel(0); // Channel 1
    #ifdef MULTI
    #ifndef DISABLE_CH_2
    _setupChannel(1);
    #endif
    #ifndef DISABLE_CH_3
    _setupChannel(2);
    #endif
    #ifndef DISABLE_CH_4
    _setupChannel(3);
    #endif
    #endif
  }

  void Hardware::postSetup() {
    blinkLED(40, 8);
    #ifdef ORIG
    setLED(getRelay(0));
    #else
    setLED(true);
    #endif
  }

  void Hardware::_setupChannel(int ch) {
    pinMode(_btnPin[ch], INPUT);
    pinMode(_relayPin[ch], OUTPUT);
    digitalWrite(_relayPin[ch], LOW);

    if (_rememberState[ch]) {
      setRelay(ch, EEPROM.read(ch));
    }

    _btnTimer[ch].attach(0.05, std::bind(&Hardware::_buttonHandler, this, ch));
  }

  void Hardware::_buttonHandler(int ch) {
    if (!digitalRead(_btnPin[ch])) {
      _btnCount[ch]++;
    } else {
      if (_btnCount[ch] > 1 && _btnCount[ch] <= 40) {
        setRelay(ch, !getRelay(ch));
      }
      else if (_btnCount[ch] > 40) {
        Serial.println("\n\nSonoff rebooting, please wait");
        _restart = true;
      }
      _btnCount[ch] = 0;
    }
  }

  void Hardware::blinkLED(int duration, int n) {
    for(int i = 0; i < n; i++)  {
      setLED(true);
      delay(duration);
      setLED(false);
      delay(duration);
    }
  }

  bool Hardware::getLED() {
    return !digitalRead(_ledPin);
  }

  void Hardware::setLED(bool on) {
    digitalWrite(_ledPin, !on); // Sonoff LED is inverted, HIGH = off
  }

  bool Hardware::getRelay(int ch) {
    return digitalRead(_relayPin[ch]);
  }

  void Hardware::setRelay(int ch, bool state) {
    #ifdef ORIG
    setLED(state);
    #endif
    digitalWrite(_relayPin[ch], state);
    setSendState(ch);

    if (_rememberState[ch]) {
      EEPROM.write(ch, state);
      EEPROM.commit();
    }
  }

  bool Hardware::getSendState(int ch) {
    return _sendState[ch];
  }

  void Hardware::setSendState(int ch) {
    _sendState[ch] = true;
  }

  bool Hardware::checkState(int ch) {
    _sendState[ch] = false;
    return getRelay(ch);
  }

  bool Hardware::restart() {
    return _restart;
  }
}
