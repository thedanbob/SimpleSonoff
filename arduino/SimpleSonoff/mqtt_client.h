#ifndef SIMPLE_SONOFF_MQTT_H
#define SIMPLE_SONOFF_MQTT_H

#include <WiFiClient.h>
#include <PubSubClient.h>
#include "config.h"

namespace SimpleSonoff {
  class MQTTClient {
    static const String version;

    #if defined(MULTI)
    static const String cmdTopic[4];
    static const String statTopic[4];
    #else
    static const String cmdTopic[1];
    static const String statTopic[1];
    #endif

    std::unique_ptr<WiFiClient> wifiClient;
    std::unique_ptr<PubSubClient> pubSubClient;
    char uid[16];
    bool restart;
    void heartbeat();

    public:
      MQTTClient(std::function<void(const MQTT::Publish&)> mqttCallback);
      char* UID();
      bool connect();
      void loop();
      bool checkAlive();
      void publishChannel(int ch, String msg);
      void publishDebug(String msg);
      void publishTemp(String msg);
      int topicToChannel(String topic);
  };
}

#endif
