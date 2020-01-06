#ifndef SIMPLE_SONOFF_MQTT_H
#define SIMPLE_SONOFF_MQTT_H

#include <memory>
#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "defines.h"
#include "hardware.h"

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
    SimpleSonoff::Hardware* hardware;
    char uid[16];
    bool restart;
    void heartbeat();
    void callback(const MQTT::Publish& pub);

    public:
      MQTTClient(SimpleSonoff::Hardware* h);
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
