#ifndef SIMPLE_SONOFF_TEMP_H
#define SIMPLE_SONOFF_TEMP_H

#include <memory>
#include <DHT.h>
#include "config.h"
#include "hardware.h"
#include "mqtt_client.h"

namespace SimpleSonoff {
  class Temp {
    bool report;
    std::unique_ptr<DHT> dht;
    SimpleSonoff::Hardware* hardware;
    SimpleSonoff::MQTTClient* mqttClient;

    public:
      Temp(SimpleSonoff::Hardware* h, SimpleSonoff::MQTTClient* m);
      void doReport();
      void reportTemp();
  };
}

#endif
