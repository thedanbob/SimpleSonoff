#include <EEPROM.h>
#include "hardware.h"

namespace SimpleSonoff {
  const int Hardware::ledPin = LED_PIN;
  #ifdef MULTI
  const int Hardware::btnPin[] = {BTN_PIN_1, BTN_PIN_2, BTN_PIN_3, BTN_PIN_4};
  const int Hardware::relayPin[] = {RELAY_PIN_1, RELAY_PIN_2, RELAY_PIN_3, RELAY_PIN_4};
  const bool Hardware::rememberState[] = {REMEMBER_RELAY_STATE_1, REMEMBER_RELAY_STATE_2, REMEMBER_RELAY_STATE_3, REMEMBER_RELAY_STATE_4};
  #else
  const int Hardware::btnPin[] = {BTN_PIN_1};
  const int Hardware::relayPin[] = {RELAY_PIN_1};
  const bool Hardware::rememberState[] = {REMEMBER_RELAY_STATE_1};
  #endif
  const String Hardware::stateName[] = {"off", "on"};

  Hardware::Hardware() {
    for (int i = 0; i < 4; i++) {
      this->relayState[i] = false;
      this->sendState[i] = false;
      this->btnCount[i] = 0;
    }
    this->restart = false;
  }

  void Hardware::setup() {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);
    EEPROM.begin(8);

    this->setupChannel(0); // Channel 1
    #ifdef MULTI
    #ifndef DISABLE_CH_2
    this->setupChannel(1);
    #endif
    #ifndef DISABLE_CH_3
    this->setupChannel(2);
    #endif
    #ifndef DISABLE_CH_4
    this->setupChannel(3);
    #endif
    #endif
  }

  void Hardware::postSetup() {
    this->blinkLED(40, 8);
    #ifdef ORIG
    digitalWrite(ledPin, !digitalRead(relayPin[0]));
    #else
    digitalWrite(ledPin, LOW);
    #endif
  }

  void Hardware::setupChannel(int ch) {
    pinMode(btnPin[ch], INPUT);
    pinMode(relayPin[ch], OUTPUT);
    digitalWrite(relayPin[ch], LOW);

    this->relayState[ch] = EEPROM.read(ch);
    if (rememberState[ch] && this->relayState[ch]) {
      #ifdef ORIG
      digitalWrite(ledPin, LOW);
      #endif
      digitalWrite(relayPin[ch], HIGH);
    }

    this->btnTimer[ch].attach(0.05, std::bind(&Hardware::buttonHandler, this, ch));
  }

  void Hardware::buttonHandler(int ch) {
    if (!digitalRead(btnPin[ch])) {
      this->btnCount[ch]++;
    } else {
      if (this->btnCount[ch] > 1 && this->btnCount[ch] <= 40) {
        #ifdef ORIG
        digitalWrite(ledPin, !digitalRead(ledPin));
        #endif
        digitalWrite(relayPin[ch], !digitalRead(relayPin[ch]));
        this->sendState[ch] = true;
      }
      else if (this->btnCount[ch] > 40) {
        Serial.println("\n\nSonoff Rebooting . . . . . . . . Please Wait");
        this->restart = true;
      }
      this->btnCount[ch] = 0;
    }
  }

  void Hardware::blinkLED(int duration, int n) {
    for(int i = 0; i < n; i++)  {
      digitalWrite(ledPin, HIGH);
      delay(duration);
      digitalWrite(ledPin, LOW);
      delay(duration);
    }
  }

  bool Hardware::getLED() {
    return !digitalRead(ledPin);
  }

  void Hardware::setLED(bool on) {
    digitalWrite(ledPin, !on); // HIGH = off
  }

  void Hardware::setRelay(int ch, bool state) {
    #ifdef ORIG
    digitalWrite(ledPin, !state);
    #endif
    digitalWrite(relayPin[ch], state);
    this->setSendState(ch);
  }

  void Hardware::setSendState(int ch) {
    this->sendState[ch] = true;
  }

  bool Hardware::shouldSendState(int ch) {
    return this->sendState[ch];
  }

  String Hardware::checkState(int ch) {
    #ifdef ORIG
    int state = this->getLED();
    #else
    int state = digitalRead(relayPin[ch]);
    #endif

    if (rememberState[ch]) {
      EEPROM.write(ch, state);
      EEPROM.commit();
    }

    Serial.print("Hardware "); Serial.print(ch + 1); Serial.print(" . . . . . . . . . . . . . . . . . . "); Serial.println(stateName[state]);
    this->sendState[ch] = false;
    return stateName[state];
  }

  void Hardware::toggleWallSwitch() {
    digitalWrite(relayPin[0], !digitalRead(relayPin[0]));
    digitalWrite(ledPin, !digitalRead(ledPin));
    this->sendState[0] = true;
  }

  bool Hardware::requestRestart() {
    return this->restart;
  }
}
