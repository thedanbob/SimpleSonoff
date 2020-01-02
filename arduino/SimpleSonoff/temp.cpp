#include <Arduino.h>
#include "temp.h"

namespace SimpleSonoff {
  Temp::Temp(SimpleSonoff::Hardware hardware, SimpleSonoff::MQTTClient mqttClient) {
    this->report = false;
    this->dht.reset(new DHT(OPT_PIN, DHTTYPE, 11));
    this->hardware.reset(&hardware);
    this->mqttClient.reset(&mqttClient);
  }

  void Temp::doReport() {
    this->report = true;
  }

  void Temp::reportTemp() {
    if (!this->report) return;

    Serial.print("DHT read . . . ");
    float dhtH, dhtT, dhtHI;

    dhtH = dht->readHumidity();
    dhtT = dht->readTemperature(USE_FAHRENHEIT);
    dhtHI = dht->computeHeatIndex(dhtT, dhtH, USE_FAHRENHEIT);

    bool ledState = hardware->getLED();
    this->hardware->blinkLED(100, 1);
    this->hardware->setLED(ledState);

    if (isnan(dhtH) || isnan(dhtT) || isnan(dhtHI)) {
      #ifdef SIMPLE_SONOFF_DEBUG
      this->mqttClient->publishDebug("\"DHT read error\"");
      #endif
      Serial.println("FAILED!");
      this->report = false;
      return;
    }

    String pubString = "{\"Temp\": "+String(dhtT)+", "+"\"Humidity\": "+String(dhtH)+", "+"\"HeatIndex\": "+String(dhtHI) + "}";
    this->mqttClient->publishTemp(pubString);
    Serial.println("ok");
    this->report = false;
  }
}