#ifndef SIMPLE_SONOFF_TEMP_H
#define SIMPLE_SONOFF_TEMP_H

#include <DHT.h>
#include "defines.h"
#include "hardware.h"
#include "mqtt_client.h"

namespace SimpleSonoff {
  class Temp {
    bool report;
    DHT dht;
    SimpleSonoff::Hardware* hardware;
    SimpleSonoff::MQTTClient* mqttClient;

    public:
      Temp(SimpleSonoff::Hardware* h, SimpleSonoff::MQTTClient* m);
      void doReport();
      void reportTemp();
  };
}

#endif
