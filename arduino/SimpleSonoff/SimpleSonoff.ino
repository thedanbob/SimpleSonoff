/*
  ===================================================================================================
    DON'T CHANGE ANYTHING BELOW UNLESS YOU KNOW WHAT YOU'RE DOING. UPDATE YOUR SETTINGS IN CONFIG.H
  ===================================================================================================
*/

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include "src/pubsubclient/PubSubClient.h"
#include "config.h"
#include "src/mqtt_client.h"
#include "src/hardware.h"


const char* header = "\n\n--------------  SimpleSonoff_v1.00  --------------";
bool restart = false;
unsigned long TasksTimer;
SimpleSonoff::Hardware hardware;
SimpleSonoff::MQTTClient mqttClient(hardware);

#ifdef WS
#include "src/wall_switch.h"
SimpleSonoff::WallSwitch wallSwitch(hardware);
#endif

#if defined(TH) && defined(TEMP)
#include "src/temp.h"
SimpleSonoff::Temp temp(hardware, mqttClient);
#endif

#ifdef ENABLE_OTA_UPDATES
#include "src/ota_update.h"
SimpleSonoff::OTAUpdate otaUpdate;
#endif

#ifdef ENABLE_HTTP_UPDATES
#include "src/http_update.h"
SimpleSonoff::HTTPUpdate httpUpdate;
#endif

void setup() {
  Serial.begin(115200);
  Serial.println(header);
  hardware.setup();

  #ifdef WS
  wallSwitch.setup();
  #endif

  if (!mqttClient.connect()) return;

  #ifdef ENABLE_OTA_UPDATES
  otaUpdate.setup(mqttClient.uid(), hardware);
  #endif

  #ifdef ENABLE_HTTP_UPDATES
  httpUpdate.setup(hardware);
  #endif

  Serial.println("\n---------------------  Logs  ---------------------\n");
  hardware.postSetup();
}

void loop() {
  #ifdef ENABLE_OTA_UPDATES
  otaUpdate.handle();
  if (otaUpdate.inProgress()) return;
  #endif

  #ifdef ENABLE_HTTP_UPDATES
  if (httpUpdate.inProgress()) return
  #endif

  mqttClient.loop();
  timedTasks();
  checkStatus();

  #ifdef WS
  wallSwitch.check();
  #endif

  #if defined(TH) && defined(TEMP)
  temp.reportTemp();
  #endif
}

void timedTasks() {
  if ((millis() > TasksTimer + (CONNECT_UPD_FREQ*60000)) || (millis() < TasksTimer)) {
    TasksTimer = millis();
    if (!mqttClient.checkAlive())
      restart = true;

    #if defined(TH) && defined(TEMP)
    temp.doReport();
    #endif

    #ifdef ENABLE_HTTP_UPDATES
    httpUpdate.checkUpdate();
    #endif
  }
}

void checkStatus() {
  mqttClient.checkChannelStatus(0);
  #ifdef MULTI
  #ifndef DISABLE_CH_2
  mqttClient.checkChannelStatus(1);
  #endif
  #ifndef DISABLE_CH_3
  mqttClient.checkChannelStatus(2);
  #endif
  #ifndef DISABLE_CH_4
  mqttClient.checkChannelStatus(3);
  #endif
  #endif

  restart |= hardware.restart();
  if (restart) {
    hardware.blinkLED(400, 4);
    ESP.restart();
  }
}
