#include <Arduino.h>

// Required by MQTTbroker library
#include <ESP8266WiFi.h>

// Required by WebSockets library
#include <Hash.h>

#include <FS.h>

#include <ArduinoLog.h>

#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <MQTTbroker.h>
#include <TaskScheduler.h>



#ifdef HARDWIRED_AP
#include "secret_hardwiredAP.h"
#endif

#define CONFIG_FILE "config.json"

int watchdog = 0;


// Configuration that we'll store on disk
struct Config {
  uint chipId;
  int  rgbColor;
  char avatar[32];
  char name[32];
};
Config config;

void loadConfiguration(const char *filename, Config &config) {
  File file = SPIFFS.open(CONFIG_FILE, "r");
  if (!file){
    // No config file exists, create one

  } else {
    size_t size = file.size();
    StaticJsonBuffer<188> jsonBuffer;
    if ( size == 0 ) {
      // Serial.println("Fichier historique vide - History file empty !");
    } else {
      std::unique_ptr<char[]> buf (new char[size]);
      file.readBytes(buf.get(), size);
      JsonObject& root = jsonBuffer.parseObject(buf.get());
      if (!root.success()) {
        //  cannot read JSON file
      } else {
        // Configuration loaded - transfer to config struct
      }
    }
  }
  file.close();
}


WebSocketsServer webSocket = WebSocketsServer(80,"","mqtt");
MQTTbroker Broker = MQTTbroker(&webSocket);

void publishClusterStatsCallback(){
  Log.notice("Publishing cluster stats via MQTT" CR );
  std::string val = "Value";
  std::string topic = "/Cluster/timestamp";
  Broker.publish(topic.c_str(), (uint8_t*)val.c_str(), val.length());
}
//Tasks
Task publishClusterStats(10000, TASK_FOREVER, &publishClusterStatsCallback);

Scheduler m_taskScheduler;

bool m_wifiInitialized = false;

void MQTTCallback(String topic_name, uint8_t * payload, uint8_t length_payload){
        Serial.printf("Receive publish to ");
        Serial.print(topic_name + " ");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            if (Broker.clientIsConnected(num)) Broker.disconnect(num);
            break;
        case WStype_BIN:
            Broker.parsing(num, payload, length);
            break;
    }
}

bool createAP(){

#ifndef HARDWIRED_AP
  int n = WiFi.scanNetworks();
  // find the strongest suitable signal
  //  * Preferred : An infrastructure AP
  //  * Otherwise : Any ChibitAP* with the strongest signal
#else
  // For now just connect to a hard coded network
  Log.notice("Connecting to hardwired access point" CR );
  WiFi.begin(AP_SSID, AP_PASS);

  int count = 0;
  int MAXCOUNT=10;
  while (count <= MAXCOUNT && WiFi.status() != WL_CONNECTED) {
    delay(500);
    count++;
  }
  if(count >=MAXCOUNT){
     Log.warning("Could not connect to hardwired access point" CR );
     return false;
  }
  m_wifiInitialized = true;
  IPAddress myIP = WiFi.localIP();

  Serial.print("AP IP address: ");
  Serial.println(myIP);

  Log.notice("Connected to hardwired access point " AP_SSID CR);
#endif
  return true;
}


void setup() {
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  Log.notice(CR CR "Starting Chibit ..." CR);

  m_taskScheduler.init();
  Log.notice(CR CR "Initialized scheduler ..." CR);

  if(!createAP()){
    Log.warning("Could not start WiFi networking !" CR);
  }else{
    Log.notice(CR CR "Starting up MQTT broker ..." CR);
    // Startup the MQTT and WebSockets loops
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    Broker.begin();
    Broker.setCallback(MQTTCallback);

    m_taskScheduler.addTask(publishClusterStats);
    publishClusterStats.enable();
  }

}

void loop() {
  m_taskScheduler.execute();
  if(m_wifiInitialized){
    webSocket.loop();
  }

}
