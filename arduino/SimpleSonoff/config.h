#ifndef SIMPLE_SONOFF_CONFIG_H
#define SIMPLE_SONOFF_CONFIG_H
/*
  ======================================================================================================================================
  Arduino upload settings (Tools menu):

  Sonoff Basic, SV, Touch, S20 Smart Socket, TH Series:
    Board: ITEAD Sonoff (select model from next dropdown)
  Sonoff 4CH, 4CH Pro & T1:
    Board: Generic ESP8285 Module
    Flash Size: 1M (64K SPIFFS)
  ======================================================================================================================================
                                            Modify all parameters below to suit your environment
  ======================================================================================================================================
*/
#define ORIG                                           // Device type: ORIG for Basic / Original Sonoff, TH for TH Series, MULTI for 4CH / 4CH Pro
//#define TEMP                                         // Uncomment for DHT11/22 support (TH only) **Must install 'DHT sensor library' (Adafruit) & 'Adafruit Unified Sensor' library.
//#define WS                                           // Uncomment for external wallswitch support

#define DHTTYPE DHT22                                  // Set to 'DHT11' or 'DHT22'. (Only applies if using TEMP) **Must connect to the mains power for temperature readings to be sent.
#define USE_FAHRENHEIT false                           // Set to 'true' to use Fahrenheit. (Only applies if using TEMP)

#define MQTT_SERVER "192.168.0.100"                    // Your mqtt server ip address
#define MQTT_PORT 1883                                 // Your mqtt port
#define MQTT_BASE_TOPIC "home/sonoff/living_room/1"    // Base mqtt topic
#define MQTT_USER "mqttUser"                           // mqtt username
#define MQTT_PASS "mqttPass"                           // mqtt password
#define MQTT_RETAIN false                              // Retain mqtt messages
#define MQTT_QOS 0                                     // QOS level for all mqtt messages. (0 or 1)

#define WIFI_SSID "wifissid"                           // Your WiFi ssid
#define WIFI_PASS "wifipass"                           // Your WiFi password
#define CONNECT_RETRIES 10                             // Number of times to retry connection to wifi / mqtt
#define CONNECT_UPD_FREQ 1                             // Number of minutes between wifi / mqtt connection check

// Remember relay states after power loss. If set to false, relay will be off when power is restored. 2-4 only apply to device type MULTI.
#define REMEMBER_RELAY_STATE_1 true
#define REMEMBER_RELAY_STATE_2 true
#define REMEMBER_RELAY_STATE_3 true
#define REMEMBER_RELAY_STATE_4 true

#define ENABLE_OTA_UPDATES                             // Enable updates via the Arduino IDE

// Uncomment below to disable MULTI channels 2-4 (channel 1 is always enabled)
//#define DISABLE_CH_2
//#define DISABLE_CH_3
//#define DISABLE_CH_4

// Uncomment below to enable debug reporting
//#define SIMPLE_SONOFF_DEBUG
/*
  ======================================================================================================================================
*/
#endif
