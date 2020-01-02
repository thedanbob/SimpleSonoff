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
#if defined(TH) && defined(TEMP)
#include "DHT.h"
#endif

const char* header = "\n\n--------------  SimpleSonoff_v1.00  --------------";
bool requestRestart = false;
unsigned long TasksTimer;
SimpleSonoff::Hardware hardware;
SimpleSonoff::MQTTClient mqttClient(hardware);

#ifdef ENABLE_OTA_UPDATES
#include "ota_update.h"
SimpleSonoff::OTAUpdate otaUpdate;
#endif

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
  Serial.println(header);
  hardware.setup();

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
  checkWallSwitch();
  #endif

  #if defined(TH) && defined(TEMP)
  getTemp();
  #endif
}

void timedTasks() {
  if ((millis() > TasksTimer + (CONNECT_UPD_FREQ*60000)) || (millis() < TasksTimer)) {
    TasksTimer = millis();
    if (!mqttClient.checkAlive())
      requestRestart = true;

    #if defined(TH) && defined(TEMP)
    tempReport = true;
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
