#ifndef SIMPLE_SONOFF_TEMP_H
#define SIMPLE_SONOFF_TEMP_H

#include <DHT.h>
#include "../defines.h"
#include "hardware.h"
#include "mqtt_client.h"

namespace SimpleSonoff {
  class Temp {
    bool _report;
    DHT _dht;
    SimpleSonoff::Hardware *_hardware;
    SimpleSonoff::MQTTClient *_mqttClient;

    public:
      Temp(SimpleSonoff::Hardware &h, SimpleSonoff::MQTTClient &m);
      void doReport();
      void reportTemp();
  };
}

#endif
