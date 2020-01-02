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

const char* header = "\n\n--------------  SimpleSonoff_v1.00  --------------";
bool requestRestart = false;
unsigned long TasksTimer;
SimpleSonoff::Hardware hardware;
SimpleSonoff::MQTTClient mqttClient(&hardware);

#ifdef WS
#include "wall_switch.h"
SimpleSonoff::WallSwitch wallSwitch(&hardware);
#endif

#if defined(TH) && defined(TEMP)
#include "temp.h"
SimpleSonoff::Temp temp(&hardware, &mqttClient);
#endif

#ifdef ENABLE_OTA_UPDATES
#include "ota_update.h"
SimpleSonoff::OTAUpdate otaUpdate;
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
  otaUpdate.setup(mqttClient.UID(), hardware);
  #endif

  Serial.println("\n---------------------  Logs  ---------------------\n");
  hardware.postSetup();
}

void loop() {
  #ifdef ENABLE_OTA_UPDATES
  otaUpdate.handle();
  if (otaUpdate.doUpdate()) return;
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
      requestRestart = true;

    #if defined(TH) && defined(TEMP)
    temp.doReport();
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

  requestRestart |= hardware.requestRestart();
  #ifdef ENABLE_OTA_UPDATES
  requestRestart |= otaUpdate.requestRestart();
  #endif

  if (requestRestart) {
    hardware.blinkLED(400, 4);
    ESP.restart();
  }
}
