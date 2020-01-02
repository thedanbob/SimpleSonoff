#include <Arduino.h>
#include <EEPROM.h>
#include <Ticker.h>
#include "hardware.h"

namespace SimpleSonoff {
  const int Hardware::ledPin = 13;
  const int Hardware::btnPin[] = {0, 9, 10, 14};
  const int Hardware::relayPin[] = {12, 5, 4, 15};
  const bool Hardware::rememberState[] = {REMEMBER_RELAY_STATE_1, REMEMBER_RELAY_STATE_2, REMEMBER_RELAY_STATE_3, REMEMBER_RELAY_STATE_4};
  const String Hardware::stateName[] = {"off", "on"};

  Hardware::Hardware() {
    for (int i = 0; i < 4; i++) {
      this->relayState[i] = false;
      this->sendState[i] = false;
      this->btnCount[i] = 0;
    }
    this->restart = false;
  }

  void Hardware::init() {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);
    EEPROM.begin(8);

    setupChannel(0); // Channel 1
    #ifdef MULTI
    #ifndef DISABLE_CH_2
    setupChannel(1);
    #endif
    #ifndef DISABLE_CH_3
    setupChannel(2);
    #endif
    #ifndef DISABLE_CH_4
    setupChannel(3);
    #endif
    #endif
  }

  void Hardware::finishInit() {
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
    if (rememberState[ch] && this->relayState[ch] == 1) {
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
