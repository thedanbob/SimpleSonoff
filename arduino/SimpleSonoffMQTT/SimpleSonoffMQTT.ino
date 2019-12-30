/*
  Copyright (c) 2019 Dan Arnfield
  Copyright (c) 2017 @KmanOz

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ======================================================================================
      ATTENTION !!!!!! DO NOT CHANGE ANYTHING BELOW. UPDATE YOUR DETAILS IN CONFIG.H
  ======================================================================================
*/

#include "config.h"
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <Ticker.h>
#if defined(TH) && defined(TEMP)
#include "DHT.h"
#endif

const char* hostPrefix = "Sonoff_%s";
const char* header     = "\n\n--------------  SimpleSonoffMQTT_v1.00  --------------";
const char* version    = "ds_v1.00";

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
char espChipID[8];
char uid[16];
long rssi;
unsigned long TasksTimer;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient, MQTT_SERVER, MQTT_PORT);

#if defined(MULTI)
const String mqttCmdTopic[] = {MQTT_BASE_TOPIC"/ch1", MQTT_BASE_TOPIC"/ch2", MQTT_BASE_TOPIC"/ch3", MQTT_BASE_TOPIC"/ch4"};
const String mqttStatTopic[] = {MQTT_BASE_TOPIC"/ch1/stat", MQTT_BASE_TOPIC"/ch2/stat", MQTT_BASE_TOPIC"/ch3/stat", MQTT_BASE_TOPIC"/ch4/stat"};
#else
const String mqttCmdTopic[] = {MQTT_BASE_TOPIC};
const String mqttStatTopic[] = {MQTT_BASE_TOPIC"/stat"};
#endif
const String mqttDebugTopic = MQTT_BASE_TOPIC"/debug";
const String mqttHeartbeatTopic = MQTT_BASE_TOPIC"/heartbeat";

#if defined(TEMP) || defined(WS)
const int optPin = 14;
#endif

#ifdef WS
int lastWallSwitch = 1;
#endif

#if defined(TH) && defined(TEMP)
DHT dht(optPin, DHTTYPE, 11);
const String mqttTempTopic = MQTT_BASE_TOPIC"/temp";
bool tempReport = false;
#endif

void setup() {
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  #ifdef WS
  pinMode(optPin, INPUT_PULLUP);
  #endif
  sprintf(espChipID, "%06X", ESP.getChipId());
  sprintf(uid, hostPrefix, espChipID);

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

  WiFi.mode(WIFI_STA);
  WiFi.hostname(uid);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  mqttClient.set_callback(mqttCallback);
  setupOTA();

  Serial.println(header);
  Serial.print("\nUnit ID: "); Serial.print(uid);
  Serial.print("\nConnecting to wifi: "); Serial.print(WIFI_SSID);

  for (int r = 0; r < CONNECT_RETRIES; r++) {
    if (WiFi.status() == WL_CONNECTED) break;
    delay(500);
    Serial.print(" .");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(" WiFi FAILED!");
    Serial.println("\n--------------------------------------------------");
    Serial.println();
    return;
  }

  Serial.println(" DONE");
  Serial.print("IP Address is: "); Serial.println(WiFi.localIP());
  Serial.print("Connecting to "); Serial.print(MQTT_SERVER); Serial.print(" Broker . .");
  delay(500);

  for (int r = 0; r < CONNECT_RETRIES; r++) {
    if (mqttClient.connect(MQTT::Connect(uid).set_keepalive(90).set_auth(MQTT_USER, MQTT_PASS))) break;
    Serial.print(" .");
    delay(1000);
  }

  if (!mqttClient.connected()) {
    Serial.println(" FAILED!");
    Serial.println("\n--------------------------------------------------");
    Serial.println();
    return;
  }

  Serial.println(" DONE");
  Serial.println("\n---------------------  Logs  ---------------------");
  Serial.println();

  mqttClient.subscribe(mqttCmdTopic[0]);
  #ifdef MULTI
  #ifndef DISABLE_CH_2
  mqttClient.subscribe(mqttCmdTopic[1]);
  #endif
  #ifndef DISABLE_CH_3
  mqttClient.subscribe(mqttCmdTopic[2]);
  #endif
  #ifndef DISABLE_CH_4
  mqttClient.subscribe(mqttCmdTopic[3]);
  #endif
  #endif

  blinkLED(led, 40, 8);
  #ifdef ORIG
  digitalWrite(led, !digitalRead(relay[0]));
  #else
  digitalWrite(led, LOW);
  #endif
}

void loop() {
  ArduinoOTA.handle();
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

void setupOTA() {
  ArduinoOTA.setHostname(uid);

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
  int index = 0;

  #ifdef MULTI
  if (pub.topic() == mqttCmdTopic[1]) index = 0;
  #ifndef DISABLE_CH_2
  else if (pub.topic() == mqttCmdTopic[1]) index = 1;
  #endif
  #ifndef DISABLE_CH_3
  else if (pub.topic() == mqttCmdTopic[2]) index = 2;
  #endif
  #ifndef DISABLE_CH_4
  else if (pub.topic() == mqttCmdTopic[3]) index = 3;
  #endif
  #endif

  mqttCmdHandler(index, pub.payload_string());
}

void mqttCmdHandler(int index, String cmd) {
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
    checkConnection();
    mqttHeartbeat();

    #if defined(TH) && defined(TEMP)
    tempReport = true;
    #endif
  }
}

void checkConnection() {
  if (WiFi.status() != WL_CONNECTED)  {
    Serial.println("WiFi connection . . . . . . . . . . LOST");
    requestRestart = true;
    return;
  }

  if (mqttClient.connected()) {
    Serial.println("mqtt broker connection . . . . . . . . . . OK");
  }
  else {
    Serial.println("mqtt broker connection . . . . . . . . . . LOST");
    requestRestart = true;
  }
}

void mqttHeartbeat() {
  #ifdef SSM_DEBUG
  rssi = WiFi.RSSI();
  char message_buff[120];
  String pubString = "{\"UID\": "+String(uid)+", "+"\"WiFi RSSI\": "+String(rssi)+"dBM"+", "+"\"Topic\": "+String(MQTT_BASE_TOPIC)+", "+"\"Ver\": "+String(version)+"}";
  pubString.toCharArray(message_buff, pubString.length()+1);
  mqttClient.publish(MQTT::Publish(mqttDebugTopic, message_buff).set_retain(MQTT_RETAIN).set_qos(MQTT_QOS));
  #endif
  mqttClient.publish(MQTT::Publish(mqttHeartbeatTopic, "OK").set_retain(MQTT_RETAIN).set_qos(MQTT_QOS));
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

  mqttClient.publish(MQTT::Publish(mqttStatTopic[index], relayStateName[state]).set_retain(MQTT_RETAIN).set_qos(MQTT_QOS));
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
  char message_buff[60];

  dhtH = dht.readHumidity();
  dhtT = dht.readTemperature(USE_FAHRENHEIT);
  dhtHI = dht.computeHeatIndex(dhtT, dhtH, USE_FAHRENHEIT);

  int ledState = digitalRead(led);
  blinkLED(led, 100, 1);
  digitalWrite(led, ledState);

  if (isnan(dhtH) || isnan(dhtT) || isnan(dhtHI)) {
    #ifdef SSM_DEBUG
    mqttClient.publish(MQTT::Publish(mqttDebugTopic,"\"DHT Read Error\"").set_retain(MQTT_RETAIN).set_qos(MQTT_QOS));
    #endif
    Serial.println("DHT read error");
    tempReport = false;
    return;
  }

  String pubString = "{\"Temp\": "+String(dhtT)+", "+"\"Humidity\": "+String(dhtH)+", "+"\"HeatIndex\": "+String(dhtHI) + "}";
  pubString.toCharArray(message_buff, pubString.length()+1);
  mqttClient.publish(MQTT::Publish(mqttTempTopic, message_buff).set_retain(MQTT_RETAIN).set_qos(MQTT_QOS));
  Serial.println("DHT read OK");
  tempReport = false;
}
#endif
