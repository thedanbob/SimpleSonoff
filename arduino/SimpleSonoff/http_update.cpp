#include <Arduino.h>
#include <ESP8266httpUpdate.h>
#include "http_update.h"

namespace SimpleSonoff {
  HTTPUpdate::HTTPUpdate() :
    update(false)
  {}

  void HTTPUpdate::setup(SimpleSonoff::Hardware &hardware) {
    ESPhttpUpdate.setLedPin(LED_PIN, LOW);

    ESPhttpUpdate.onStart([this, &hardware]() {
      this->update = true;
      hardware.blinkLED(400, 2);
      hardware.setLED(false);
      Serial.println("HTTP update initiated...");
    });

    ESPhttpUpdate.onEnd([this]() {
      Serial.println("HTTP update done");
      this->update = false;
    });

    ESPhttpUpdate.onProgress([&hardware](int progress, int total) {
      hardware.setLED(true);
      delay(5);
      hardware.setLED(false);
      Serial.printf("Progress: %i%%\r", (progress / (total / 100)));
    });

    ESPhttpUpdate.onError([this, &hardware](int error) {
      hardware.blinkLED(40, 2);
      this->update = false;
      Serial.printf("HTTP error: [%i] ", error);
      Serial.println(ESPhttpUpdate.getLastErrorString());
    });
  }

  void HTTPUpdate::checkUpdate() {
    ESPhttpUpdate.update(HTTP_UPDATE_HOST, HTTP_UPDATE_PORT, HTTP_UPDATE_PATH);
  }

  bool HTTPUpdate::inProgress() {
    return this->update;
  }
}
