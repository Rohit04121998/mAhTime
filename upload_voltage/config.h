#include <ESP8266WiFi.h> // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> // Allows us to connect to, and publish to the MQTT broker


// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "Your SSID";
const char* wifi_password = "Your WiFi_Password";

// MQTT
// Make sure to update this for your own MQTT Broker!
const char* mqtt_server = "MQTT_Server IP";
const char* mqtt_topic = "YourTopicName";
const char* mqtt_username = "MQTT_Ussername";
const char* mqtt_password = "Password of your username if given any";
// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "Greg";


// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient); // 1883 is the listener port for the Broker
