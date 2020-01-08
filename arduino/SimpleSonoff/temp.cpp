#include <Arduino.h>
#include "temp.h"

namespace SimpleSonoff {
  Temp::Temp(SimpleSonoff::Hardware &h, SimpleSonoff::MQTTClient &m) :
    _report(false),
    _dht(OPT_PIN, DHTTYPE, 11),
    _hardware(&h),
    _mqttClient(&m)
  {}

  void Temp::doReport() {
    _report = true;
  }

  void Temp::reportTemp() {
    if (!_report) return;

    Serial.print("DHT read . . . ");
    float dhtH, dhtT, dhtHI;

    dhtH = _dht.readHumidity();
    dhtT = _dht.readTemperature(USE_FAHRENHEIT);
    dhtHI = _dht.computeHeatIndex(dhtT, dhtH, USE_FAHRENHEIT);

    bool ledState = _hardware->getLED();
    _hardware->blinkLED(100, 1);
    _hardware->setLED(ledState);

    if (isnan(dhtH) || isnan(dhtT) || isnan(dhtHI)) {
      #ifdef SIMPLE_SONOFF_DEBUG
      _mqttClient->publishDebug("\"DHT read error\"");
      #endif
      Serial.println("FAILED!");
      _report = false;
      return;
    }

    String pubString = "{\"Temp\": "+String(dhtT)+", "+"\"Humidity\": "+String(dhtH)+", "+"\"HeatIndex\": "+String(dhtHI) + "}";
    _mqttClient->publishTemp(pubString);
    Serial.println("ok");
    _report = false;
  }
}
