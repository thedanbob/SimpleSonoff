#include <Arduino.h>
#include <ArduinoOTA.h>
#include "ota_update.h"

namespace SimpleSonoff {
  OTAUpdate::OTAUpdate() :
    _inProgress(false)
  {}

  void OTAUpdate::setup(char uid[16], SimpleSonoff::Hardware &hardware) {
    ArduinoOTA.setHostname(uid);

    ArduinoOTA.onStart([this, &hardware]() {
      _inProgress = true;
      hardware.blinkLED(400, 2);
      hardware.setLED(false);
      Serial.println("OTA update initiated...");
    });

    ArduinoOTA.onEnd([this]() {
      Serial.println("\nOTA update done");
      _inProgress = false;
    });

    ArduinoOTA.onProgress([&hardware](unsigned int progress, unsigned int total) {
      hardware.setLED(true);
      delay(5);
      hardware.setLED(false);
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([this, &hardware](ota_error_t error) {
      hardware.blinkLED(40, 2);
      _inProgress = false;
      Serial.printf("OTA error: [%u] ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
  }

  void OTAUpdate::handle() {
    ArduinoOTA.handle();
  }

  bool OTAUpdate::inProgress() {
    return _inProgress;
  }
}
