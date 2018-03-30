#include <Arduino.h>

// Required by MQTTbroker library
#include <ESP8266WiFi.h>

// Required by WebSockets library
#include <Hash.h>

#include <FS.h>

#include <ArduinoLog.h>

#include <ArduinoJson.h>
#include <TaskScheduler.h>

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <ESP8266mDNS.h>

#include "uMQTTBroker.h"

///////////////////////////////
// project-level headers
#include "globals.h"

#include "wifi.h"



//////////////////////


#define CONFIG_FILE "config.json"

int watchdog = 0;


AsyncWebServer httpServer(80);
AsyncWebSocket ws("/ws");


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


//WebSocketsServer webSocket = WebSocketsServer(80,"","mqtt");
//MQTTbroker Broker = MQTTbroker(&webSocket);

void publishClusterStatsCallback(){
  Log.notice("Publishing cluster stats via MQTT" CR );
  std::string cid = String(ESP.getChipId()).c_str();
  std::string val = "{'host':" +cid +"}";
  std::string topic = "/Cluster/timestamp";
  std::string wsMsg = "{'topic':'"+topic+"', 'value': "+val+"}";

  MQTT_local_publish((unsigned char *)"/Cluster/timestamp", (unsigned char *)val.c_str(), val.length(), 0, 0);
  ws.textAll(wsMsg.c_str());
}

//Tasks
Task publishClusterStats(10000, TASK_FOREVER, &publishClusterStatsCallback);

Scheduler m_taskScheduler;

bool m_wifiInitialized = false;

void MQTTCallback(uint32_t *client, const char* topic, uint32_t topic_len, const char *data, uint32_t lengh) {
  char topic_str[topic_len+1];
  os_memcpy(topic_str, topic, topic_len);
  topic_str[topic_len] = '\0';

  char data_str[lengh+1];
  os_memcpy(data_str, data, lengh);
  data_str[lengh] = '\0';

  Serial.print("received topic '");
  Serial.print(topic_str);
  Serial.print("' with data '");
  Serial.print(data_str);
  Serial.println("'");
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
 if(type == WS_EVT_CONNECT){
   Log.notice("ws[%s][%u] connect" CR, server->url(), client->id());
   client->printf("Hello Client %u :)", client->id());
   client->ping();
 } else if(type == WS_EVT_DISCONNECT){
   //Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
 } else if(type == WS_EVT_ERROR){
   Log.warning("ws[%s][%u] error(%u): %s" CR, server->url(), client->id(), *((uint16_t*)arg), (char*)data);
 } else if(type == WS_EVT_PONG){
   //Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
 } else if(type == WS_EVT_DATA){
   AwsFrameInfo * info = (AwsFrameInfo*)arg;
   String msg = "";
   if(info->final && info->index == 0 && info->len == len){
     //the whole message is in a single frame and we got all of it's data
     Log.notice("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

     if(info->opcode == WS_TEXT){
       for(size_t i=0; i < info->len; i++) {
         msg += (char) data[i];
       }
     } else {
       char buff[3];
       for(size_t i=0; i < info->len; i++) {
         sprintf(buff, "%02x ", (uint8_t) data[i]);
         msg += buff ;
       }
     }
     Log.notice("%s" CR,msg.c_str());

     if(info->opcode == WS_TEXT)
       client->text("I got your text message");
     else
       client->binary("I got your binary message");
   } else {
     // We don't handle multiframe messages
     Log.warning("ws - multiframe messages not handled !" CR);
   }
 }
}


void setup() {
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  Log.notice(CR CR "Starting Chibit ..." CR);

  m_taskScheduler.init();
  Log.notice(CR CR "Initialized scheduler ..." CR);
  std::string cid = String(ESP.getChipId()).c_str();

  m_wifiInitialized = wifi_joinOrCreateAP("chibit"); // + cid);

  if(!m_wifiInitialized){
    Log.fatal("Could not start WiFi networking !" CR);
  }else{
    Log.notice(CR CR "Starting file system ..." CR);
    SPIFFS.begin();

    Log.notice(CR CR "Starting up Web server ..." CR);

    MDNS.addService("http","tcp",80);
    // Startup the MQTT and WebSockets loops
    //webSocket.begin();
    //webSocket.onEvent(webSocketEvent);
    ws.onEvent(onWsEvent);
    httpServer.addHandler(&ws);
    httpServer.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", String(ESP.getFreeHeap()));
      });
    httpServer.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
    httpServer.begin();

    Log.notice(CR CR "Starting up MQTT broker ..." CR);
    MDNS.addService("mqtt","tcp",MQTT_PORT);

    MQTT_server_onData(MQTTCallback);
    MQTT_server_start(MQTT_PORT, 30, 30);

    m_taskScheduler.addTask(publishClusterStats);
    publishClusterStats.enable();

  }

}

void loop() {
  if(m_wifiInitialized){
    m_taskScheduler.execute();
    // webSocket.loop();

  }

}
