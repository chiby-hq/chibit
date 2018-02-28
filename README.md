# chibit
A connected bracelet for Chiby based on ESP8266


# Arduino compilation notes

To compile the code, you need to specify the following symbols :

WIFI_SSID
WIFI_PASSWORD

You can also specify the following symbols to override defaults :

WIFI_CHANNEL
MQTT_PORT
MESH_PREFIX
MESH_PASSWORD
MESH_PORT

You can create a "secrets.h" file in the sketch folder to define above symbols.