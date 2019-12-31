/*
  ======================================================================================
      ATTENTION !!!!!! DO NOT CHANGE ANYTHING BELOW. UPDATE YOUR DETAILS IN CONFIG.H
  ======================================================================================
*/

#include "config.h"
#include "mqtt_client.h"
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#ifdef ENABLE_OTA_UPDATES
#include <ArduinoOTA.h>
#endif
#if defined(TH) && defined(TEMP)
#include "DHT.h"
#endif

const char* header     = "\n\n--------------  SimpleSonoff_v1.00  --------------";

// The ORIG and TH variants have more memory than MULTI anyway so let's just define the multi-channel arrays
const int btn[] = {0, 9, 10, 14};
const int relay[] = {12, 5, 4, 15};
const int led = 13;

const bool rememberRelayState[] = {REMEMBER_RELAY_STATE_1, REMEMBER_RELAY_STATE_2, REMEMBER_RELAY_STATE_3, REMEMBER_RELAY_STATE_4};
int relayState[4];
const String relayStateName[] = {"off", "on"};

bool sendStatus[] = {false, false, false, false};
unsigned long btnCount[] = {0, 0, 0, 0};
Ticker btnTimer[4];

bool requestRestart = false;
bool OTAupdate = false;
unsigned long TasksTimer;
SimpleSonoff::MQTTClient mqttClient(mqttCallback);

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
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  #ifdef WS
  pinMode(optPin, INPUT_PULLUP);
  #endif

  Serial.begin(115200);
  EEPROM.begin(8);

  setupChannel(0); // Channel 1

  #ifdef MULTI
  #ifndef DISABLE_CH_2
  setupChannel(1);
  #endif
  #ifndef DISABLE_CH_3
  setupChannel(2);
  #endif
  #ifndef DISABLE_CH_4
  setupChannel(3);
  #endif
  #endif

  Serial.println(header);
  if (!mqttClient.connect()) return;

  #ifdef ENABLE_OTA_UPDATES
  setupOTA();
  #endif

  Serial.println(" DONE");
  Serial.println("\n---------------------  Logs  ---------------------");
  Serial.println();

  blinkLED(led, 40, 8);
  #ifdef ORIG
  digitalWrite(led, !digitalRead(relay[0]));
  #else
  digitalWrite(led, LOW);
  #endif
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

void setupChannel(int index) {
  pinMode(btn[index], INPUT);
  pinMode(relay[index], OUTPUT);
  digitalWrite(relay[index], LOW);

  relayState[index] = EEPROM.read(index);
  if (rememberRelayState[index] && relayState[index] == 1) {
    #ifdef ORIG
    digitalWrite(led, LOW);
    #endif
    digitalWrite(relay[index], HIGH);
  }

  btnTimer[index].attach(0.05, std::bind(buttonHandler, index));
}

#ifdef ENABLE_OTA_UPDATES
void setupOTA() {
  ArduinoOTA.setHostname(mqttClient.UID());

  ArduinoOTA.onStart([]() {
    OTAupdate = true;
    blinkLED(led, 400, 2);
    digitalWrite(led, HIGH);
    Serial.println("OTA Update Initiated . . .");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA Update Ended . . .s");
    OTAupdate = false;
    requestRestart = true;
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    digitalWrite(led, LOW);
    delay(5);
    digitalWrite(led, HIGH);
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    blinkLED(led, 40, 2);
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

void buttonHandler(int index) {
  if (!digitalRead(btn[index])) {
    btnCount[index]++;
  } else {
    if (btnCount[index] > 1 && btnCount[index] <= 40) {
      #ifdef ORIG
      digitalWrite(led, !digitalRead(led));
      #endif
      digitalWrite(relay[index], !digitalRead(relay[index]));
      sendStatus[index] = true;
    }
    else if (btnCount[index] > 40) {
      Serial.println("\n\nSonoff Rebooting . . . . . . . . Please Wait");
      requestRestart = true;
    }
    btnCount[index] = 0;
  }
}

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
    #ifdef ORIG
    digitalWrite(led, LOW);
    #endif
    digitalWrite(relay[index], HIGH);
  } else if (cmd == "off") {
    #ifdef ORIG
    digitalWrite(led, HIGH);
    #endif
    digitalWrite(relay[index], LOW);
  } else {
    return; // Ignore other commands
  }

  sendStatus[index] = true;
}

void blinkLED(int pin, int duration, int n) {
  for(int i = 0; i < n; i++)  {
    digitalWrite(pin, HIGH);
    delay(duration);
    digitalWrite(pin, LOW);
    delay(duration);
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

  if (requestRestart) {
    blinkLED(led, 400, 4);
    ESP.restart();
  }
}

void checkChannelStatus(int index) {
  if (!sendStatus[index]) return;

  #ifdef ORIG
  int state = !digitalRead(led);
  #else
  int state = digitalRead(relay[index]);
  #endif

  if (rememberRelayState[index]) {
    EEPROM.write(index, state);
    EEPROM.commit();
  }

  mqttClient.publishChannel(index, relayStateName[state]);
  Serial.print("Relay "); Serial.print(index); Serial.print(" . . . . . . . . . . . . . . . . . . "); Serial.println(relayStateName[state]);
  sendStatus[index] = false;
}

#ifdef WS
void checkWallSwitch() {
  int wallSwitch = digitalRead(optPin);
  if (wallSwitch != lastWallSwitch) {
    digitalWrite(relay[0], !digitalRead(relay[0]));
    digitalWrite(led, !digitalRead(led));
    sendStatus[0] = true;
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

  int ledState = digitalRead(led);
  blinkLED(led, 100, 1);
  digitalWrite(led, ledState);

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
