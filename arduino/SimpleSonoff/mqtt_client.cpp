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

  MQTTClient::MQTTClient(SimpleSonoff::Hardware &h) :
    restart(false),
    wifiClient(),
    pubSubClient(this->wifiClient, MQTT_SERVER, MQTT_PORT),
    hardware(&h)
  {
    this->pubSubClient.set_callback([this](const MQTT::Publish& pub){
      this->callback(pub);
    });
    sprintf(this->uid, "Sonoff_%06X", ESP.getChipId());
  }

  char* MQTTClient::UID() {
    return this->uid;
  }

  bool MQTTClient::connect() {
    WiFi.mode(WIFI_STA);
    WiFi.hostname(this->uid);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.print("\nUnit ID: "); Serial.println(this->uid);
    Serial.print("Connecting to wifi: "); Serial.print(WIFI_SSID);

    for (int r = 0; r < CONNECT_RETRIES; r++) {
      if (WiFi.status() == WL_CONNECTED) break;
      delay(500);
      Serial.print(". ");
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("FAILED!");
      return false;
    }

    Serial.println("done");
    Serial.print("IP Address is "); Serial.println(WiFi.localIP());
    Serial.print("Connecting to MQTT broker "); Serial.print(MQTT_SERVER); Serial.print(": ");
    delay(500);

    for (int r = 0; r < CONNECT_RETRIES; r++) {
      if (this->pubSubClient.connect(MQTT::Connect(this->uid).set_keepalive(90).set_auth(MQTT_USER, MQTT_PASS))) break;
      Serial.print(". ");
      delay(1000);
    }

    if (!this->pubSubClient.connected()) {
      Serial.println("FAILED!");
      return false;
    }

    Serial.println("done");
    Serial.print("Subscribing to topic "); Serial.println(cmdTopic[0]);
    MQTT::Subscribe subs(cmdTopic[0], MQTT_QOS);
    #ifdef MULTI
    #ifndef DISABLE_CH_2
    Serial.print("Subscribing to topic "); Serial.println(cmdTopic[0]);
    subs.add_topic(cmdTopic[1], MQTT_QOS);
    #endif
    #ifndef DISABLE_CH_3
    Serial.print("Subscribing to topic "); Serial.println(cmdTopic[0]);
    subs.add_topic(cmdTopic[2], MQTT_QOS);
    #endif
    #ifndef DISABLE_CH_4
    Serial.print("Subscribing to topic "); Serial.println(cmdTopic[0]);
    subs.add_topic(cmdTopic[3], MQTT_QOS);
    #endif
    #endif
    this->pubSubClient.subscribe(subs);

    return true;
  }

  void MQTTClient::loop() {
    this->pubSubClient.loop();
  }

  bool MQTTClient::checkAlive() {
    if (WiFi.status() != WL_CONNECTED)  {
      Serial.println("Wifi connection lost!");
      return false;
    }

    if (this->pubSubClient.connected()) {
      Serial.println("MQTT broker connection ok");
      this->heartbeat();
    }
    else {
      Serial.println("MQTT broker connection lost!");
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
    this->pubSubClient.publish(MQTT::Publish(MQTT_BASE_TOPIC"/heartbeat", "OK").set_retain(MQTT_RETAIN).set_qos(MQTT_QOS));
  }

  void MQTTClient::checkChannelStatus(int ch) {
    if (!this->hardware->getSendState(ch)) return;
    String state = this->hardware->checkState(ch) ? "on" : "off";
    Serial.print("Channel "); Serial.print(ch + 1); Serial.println(" " + state);
    this->pubSubClient.publish(MQTT::Publish(statTopic[ch], state).set_retain(MQTT_RETAIN).set_qos(MQTT_QOS));
  }

  void MQTTClient::publishDebug(String msg) {
    this->pubSubClient.publish(MQTT::Publish(MQTT_BASE_TOPIC"/debug", msg).set_retain(MQTT_RETAIN).set_qos(MQTT_QOS));
  }

  void MQTTClient::publishTemp(String msg) {
    this->pubSubClient.publish(MQTT::Publish(MQTT_BASE_TOPIC"/temp", msg).set_retain(MQTT_RETAIN).set_qos(MQTT_QOS));
  }

  void MQTTClient::callback(const MQTT::Publish& pub) {
    String cmd = pub.payload_string();
    if (cmd == "reset") {
      this->restart = true;
      return;
    }

    int ch = 0;
    String topic = pub.topic();
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

    if (cmd == "stat") {
      this->hardware->setSendState(ch);
    } else if (cmd == "on") {
      this->hardware->setRelay(ch, true);
    } else if (cmd == "off") {
      this->hardware->setRelay(ch, false);
    } else {
      return; // Ignore other commands
    }
  }
}
