#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "mqtt_client.h"

namespace SimpleSonoff {
  const String MQTTClient::version = "ss_v1.00";

  #if defined(MULTI)
  const String MQTTClient::cmdTopic[] = {MQTT_BASE_TOPIC"/ch1", MQTT_BASE_TOPIC"/ch2", MQTT_BASE_TOPIC"/ch3", MQTT_BASE_TOPIC"/ch4"};
  const String MQTTClient::statTopic[] = {MQTT_BASE_TOPIC"/ch1/stat", MQTT_BASE_TOPIC"/ch2/stat", MQTT_BASE_TOPIC"/ch3/stat", MQTT_BASE_TOPIC"/ch4/stat"};
  #else
  const String MQTTClient::cmdTopic[] = {MQTT_BASE_TOPIC};
  const String MQTTClient::statTopic[] = {MQTT_BASE_TOPIC"/stat"};
  #endif

  MQTTClient::MQTTClient(std::function<void(const MQTT::Publish&)> mqttCallback) {
    this->wifiClient.reset(new WiFiClient());
    this->pubSubClient.reset(new PubSubClient(*this->wifiClient, MQTT_SERVER, MQTT_PORT));
    this->pubSubClient->set_callback(mqttCallback);
    sprintf(this->uid, "Sonoff_%06X", ESP.getChipId());
  }

  char* MQTTClient::UID() {
    return this->uid;
  }

  bool MQTTClient::connect() {
    WiFi.mode(WIFI_STA);
    WiFi.hostname(this->uid);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.print("\nUnit ID: "); Serial.print(this->uid);
    Serial.print("\nConnecting to wifi: "); Serial.print(WIFI_SSID);

    for (int r = 0; r < CONNECT_RETRIES; r++) {
      if (WiFi.status() == WL_CONNECTED) break;
      delay(500);
      Serial.print(" .");
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(" Wifi FAILED!");
      Serial.println("\n--------------------------------------------------");
      Serial.println();
      return false;
    }

    Serial.println(" DONE");
    Serial.print("IP Address is: "); Serial.println(WiFi.localIP());
    Serial.print("Connecting to "); Serial.print(MQTT_SERVER); Serial.print(" MQTT broker . .");
    delay(500);

    for (int r = 0; r < CONNECT_RETRIES; r++) {
      if (this->pubSubClient->connect(MQTT::Connect(this->uid).set_keepalive(90).set_auth(MQTT_USER, MQTT_PASS))) break;
      Serial.print(" .");
      delay(1000);
    }

    if (!this->pubSubClient->connected()) {
      Serial.println(" FAILED!");
      Serial.println("\n--------------------------------------------------");
      Serial.println();
      return false;
    }

    this->pubSubClient->subscribe(cmdTopic[0]);
    #ifdef MULTI
    #ifndef DISABLE_CH_2
    this->pubSubClient->subscribe(cmdTopic[1]);
    #endif
    #ifndef DISABLE_CH_3
    this->pubSubClient->subscribe(cmdTopic[2]);
    #endif
    #ifndef DISABLE_CH_4
    this->pubSubClient->subscribe(cmdTopic[3]);
    #endif
    #endif

    return true;
  }

  void MQTTClient::loop() {
    this->pubSubClient->loop();
  }

  bool MQTTClient::alive() {
    if (WiFi.status() != WL_CONNECTED)  {
      Serial.println("Wifi connection . . . . . . . . . . LOST");
      return false;
    }

    if (this->pubSubClient->connected()) {
      Serial.println("MQTT broker connection . . . . . . . . . . OK");
    }
    else {
      Serial.println("MQTT broker connection . . . . . . . . . . LOST");
      return false;
    }

    return true;
  }

  void MQTTClient::heartbeat() {
    #ifdef SIMPLE_SONOFF_DEBUG
    rssi = WiFi.RSSI();
    String pubString = "{\"UID\": "+String(this->uid)+", "+"\"Wifi RSSI\": "+String(rssi)+"dBM"+", "+"\"Topic\": "+String(MQTT_BASE_TOPIC)+", "+"\"Version\": "+version+"}";
    this->publishDebug(pubString);
    #endif
    this->pubSubClient->publish(MQTT::Publish(MQTT_BASE_TOPIC"/heartbeat", "OK").set_retain(MQTT_RETAIN).set_qos(MQTT_QOS));
  }

  void MQTTClient::publishChannel(int ch, String msg) {
    this->pubSubClient->publish(MQTT::Publish(statTopic[ch], msg).set_retain(MQTT_RETAIN).set_qos(MQTT_QOS));
  }

  void MQTTClient::publishDebug(String msg) {
    this->pubSubClient->publish(MQTT::Publish(MQTT_BASE_TOPIC"/debug", msg).set_retain(MQTT_RETAIN).set_qos(MQTT_QOS));
  }

  void MQTTClient::publishTemp(String msg) {
    this->pubSubClient->publish(MQTT::Publish(MQTT_BASE_TOPIC"/temp", msg).set_retain(MQTT_RETAIN).set_qos(MQTT_QOS));
  }

  int MQTTClient::topicToChannel(String topic) {
    int ch = 0;

    #ifdef MULTI
    if (topic == cmdTopic[1]) ch = 0;
    #ifndef DISABLE_CH_2
    else if (topic == cmdTopic[1]) ch = 1;
    #endif
    #ifndef DISABLE_CH_3
    else if (topic == cmdTopic[2]) ch = 2;
    #endif
    #ifndef DISABLE_CH_4
    else if (topic == cmdTopic[3]) ch = 3;
    #endif
    #endif

    return ch;
  }
}
