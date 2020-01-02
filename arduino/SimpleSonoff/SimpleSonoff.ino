/*
  ======================================================================================
      ATTENTION !!!!!! DO NOT CHANGE ANYTHING BELOW. UPDATE YOUR DETAILS IN CONFIG.H
  ======================================================================================
*/

#include "config.h"
#include "mqtt_client.h"
#include "hardware.h"
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#ifdef ENABLE_OTA_UPDATES
#include <ArduinoOTA.h>
#endif
#if defined(TH) && defined(TEMP)
#include "DHT.h"
#endif

const char* header     = "\n\n--------------  SimpleSonoff_v1.00  --------------";

bool requestRestart = false;
bool OTAupdate = false;
unsigned long TasksTimer;
SimpleSonoff::MQTTClient mqttClient(mqttCallback);
SimpleSonoff::Hardware hardware;

#if defined(TEMP) || defined(WS)
const int optPin = 14;
#endif

#ifdef WS
int lastWallSwitch = 1;
#endif

#if defined(TH) && defined(TEMP)
DHT dht(optPin, DHTTYPE, 11);
bool tempReport = false;
#endif

void setup() {
  #ifdef WS
  pinMode(optPin, INPUT_PULLUP);
  #endif

  Serial.begin(115200);
  hardware.init();

  Serial.println(header);
  if (!mqttClient.connect()) return;

  #ifdef ENABLE_OTA_UPDATES
  setupOTA();
  #endif

  Serial.println(" DONE");
  Serial.println("\n---------------------  Logs  ---------------------");
  Serial.println();

  hardware.finishInit();
}

void loop() {
  #ifdef ENABLE_OTA_UPDATES
  ArduinoOTA.handle();
  #endif

  if (OTAupdate == false) {
    mqttClient.loop();
    timedTasks();
    checkStatus();

    #ifdef WS
    checkWallSwitch();
    #endif

    #if defined(TH) && defined(TEMP)
    getTemp();
    #endif
  }
}

#ifdef ENABLE_OTA_UPDATES
void setupOTA() {
  ArduinoOTA.setHostname(mqttClient.UID());

  ArduinoOTA.onStart([]() {
    OTAupdate = true;
    hardware.blinkLED(400, 2);
    hardware.setLED(false);
    Serial.println("OTA Update Initiated . . .");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA Update Ended . . .s");
    OTAupdate = false;
    requestRestart = true;
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    hardware.setLED(true);
    delay(5);
    hardware.setLED(false);
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    hardware.blinkLED(40, 2);
    OTAupdate = false;
    Serial.printf("OTA Error [%u] ", error);
    if (error == OTA_AUTH_ERROR) Serial.println(". . . . . . . . . . . . . . . Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println(". . . . . . . . . . . . . . . Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println(". . . . . . . . . . . . . . . Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println(". . . . . . . . . . . . . . . Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println(". . . . . . . . . . . . . . . End Failed");
  });

  ArduinoOTA.begin();
}
#endif

void mqttCallback(const MQTT::Publish& pub) {
  int index = mqttClient.topicToChannel(pub.topic());
  String cmd = pub.payload_string();

  if (cmd == "reset") {
    requestRestart = true;
    return;
  }

  if (cmd == "stat") {
    // Skip to the end
  } else if (cmd == "on") {
    hardware.setRelay(index, true);
  } else if (cmd == "off") {
    hardware.setRelay(index, false);
  } else {
    return; // Ignore other commands
  }
}

void timedTasks() {
  if ((millis() > TasksTimer + (CONNECT_UPD_FREQ*60000)) || (millis() < TasksTimer)) {
    TasksTimer = millis();
    if (mqttClient.alive()) {
      mqttClient.heartbeat();
    } else {
      requestRestart = true;
    }

    #if defined(TH) && defined(TEMP)
    tempReport = true;
    #endif
  }
}

void checkStatus() {
  checkChannelStatus(0);
  #ifdef MULTI
  #ifndef DISABLE_CH_2
  checkChannelStatus(1);
  #endif
  #ifndef DISABLE_CH_3
  checkChannelStatus(2);
  #endif
  #ifndef DISABLE_CH_4
  checkChannelStatus(3);
  #endif
  #endif

  if (requestRestart || hardware.requestRestart()) {
    hardware.blinkLED(400, 4);
    ESP.restart();
  }
}

void checkChannelStatus(int index) {
  if (!hardware.shouldSendState(index)) return;
  mqttClient.publishChannel(index, hardware.checkState(index));
}

#ifdef WS
void checkWallSwitch() {
  int wallSwitch = digitalRead(optPin);
  if (wallSwitch != lastWallSwitch) {
    hardware.toggleWallSwitch();
  }
  lastWallSwitch = wallSwitch;
}
#endif

#if defined(TH) && defined(TEMP)
void getTemp() {
  if (!tempReport) return;

  Serial.print("DHT read . . . . . . . . . . . . . . . . . ");
  float dhtH, dhtT, dhtHI;

  dhtH = dht.readHumidity();
  dhtT = dht.readTemperature(USE_FAHRENHEIT);
  dhtHI = dht.computeHeatIndex(dhtT, dhtH, USE_FAHRENHEIT);

  bool ledState = hardware.getLED();
  hardware.blinkLED(100, 1);
  hardware.setLED(ledState);

  if (isnan(dhtH) || isnan(dhtT) || isnan(dhtHI)) {
    #ifdef SIMPLE_SONOFF_DEBUG
    mqttClient.publishDebug("\"DHT Read Error\"");
    #endif
    Serial.println("DHT read error");
    tempReport = false;
    return;
  }

  String pubString = "{\"Temp\": "+String(dhtT)+", "+"\"Humidity\": "+String(dhtH)+", "+"\"HeatIndex\": "+String(dhtHI) + "}";
  mqttClient.publishTemp(pubString);
  Serial.println("DHT read OK");
  tempReport = false;
}
#endif
