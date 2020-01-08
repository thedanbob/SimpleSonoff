#ifndef SIMPLE_SONOFF_MQTT_H
#define SIMPLE_SONOFF_MQTT_H

#include <Arduino.h>
#include <WiFiClient.h>
#include "pubsubclient/PubSubClient.h"
#include "defines.h"
#include "hardware.h"

namespace SimpleSonoff {
  class MQTTClient {
    static const String _version;
    static const String _cmdTopic[CHANNELS];
    static const String _statTopic[CHANNELS];

    WiFiClient _wifiClient;
    PubSubClient _pubSubClient;
    SimpleSonoff::Hardware *_hardware;

    char _uid[16];
    bool _restart;
    void _heartbeat();
    void _callback(const MQTT::Publish &pub);

    public:
      MQTTClient(SimpleSonoff::Hardware &h);
      char* uid();
      bool connect();
      void loop();
      bool checkAlive();
      void checkChannelStatus(int ch);
      void publishDebug(String msg);
      void publishTemp(String msg);
  };
}

#endif
