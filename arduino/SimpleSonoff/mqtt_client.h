#ifndef SIMPLE_SONOFF_MQTT_H
#define SIMPLE_SONOFF_MQTT_H

#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "defines.h"
#include "hardware.h"

namespace SimpleSonoff {
  class MQTTClient {
    static const String version;
    static const String cmdTopic[CHANNELS];
    static const String statTopic[CHANNELS];

    WiFiClient wifiClient;
    PubSubClient pubSubClient;
    SimpleSonoff::Hardware *hardware;
    char uid[16];
    bool restart;
    void heartbeat();
    void callback(const MQTT::Publish &pub);

    public:
      MQTTClient(SimpleSonoff::Hardware &h);
      char* UID();
      bool connect();
      void loop();
      bool checkAlive();
      void checkChannelStatus(int ch);
      void publishDebug(String msg);
      void publishTemp(String msg);
  };
}

#endif
