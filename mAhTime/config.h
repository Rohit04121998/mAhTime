#include <ESP8266WiFi.h> // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> // Allows us to connect to, and publish to the MQTT broker


// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "Swaroop";
const char* wifi_password = "1234567890";

// MQTT
// Make sure to update this for your own MQTT Broker!
const char* mqtt_server = "192.168.43.208";
const char* mqtt_topic = "Appiko/mAhTime/BatCap";
const char* mqtt_username = "appiko";
const char* mqtt_password = "appiko@123";
// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "swaroop";


// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient); // 1883 is the listener port for the Broker
