/*
  ======================================================================================================================================
                                            Modify all parameters below to suit you environment
  ======================================================================================================================================
*/
const bool rememberRelayState1 = true;                        // Remember the state of relay 1 before power loss.
const bool rememberRelayState2 = true;                        // Remember the state of relay 2 before power loss.
const bool rememberRelayState3 = true;                        // Remember the state of relay 3 before power loss.
const bool rememberRelayState4 = true;                        // Remember the state of relay 4 before power loss.
                                                              // Each relay will be OFF evey time power is applied when set to 'false'

const bool mqttRetain = false;                                // Retain mqtt messages
const int connectUpdateFreq = 1;                                       // Update frequency in Mintes to check for mqtt connection. Defualt 1 min.
int kRetries = 10;                                            // WiFi retry count (10 default). Increase if not connecting to your WiFi.
const int QOS = 0;                                            // QOS level for all mqtt messages. (0 or 1)

                                                              // Channel 1 is always enabled
//#define CH_2                                                  // Channel 2 (Uncomment to use)
//#define CH_3                                                  // Channel 3 (Uncomment to use)
//#define CH_4                                                  // Channel 4 (Uncomment to use)

const char* mqttServer = "192.168.0.100";                     // Your mqtt server ip address
const int mqttPort = 1883;                                    // Your mqtt port
const char* mqttBaseTopic = "home/sonoff/living_room/1";      // Base mqtt topic
const char* mqttUser = "mqttUser";                            // mqtt username
const char* mqttPass = "mqttPass";                            // mqtt password

const char* ssid = "wifissid";                                // Your WiFi ssid
const char* pass = "wifipass";                                // Your WiFi password
/*
  ======================================================================================================================================
*/
