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

#include "MQTT.h"
#include "uMQTTBroker.h"

///////////////////////////////
// project-level headers
#include "globals.h"

#include "wifi.h"

//
//////////////////////

#define CONFIG_FILE "config.json"

AsyncWebServer m_httpServer(80);
AsyncWebSocket m_websocketServer("/ws");

String m_hostname;
IPAddress m_mqttBrokerIp;
uint16_t m_mqttBrokerPort = DEFAULT_MQTT_PORT;

MQTT* m_mqttClient;


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
     Log.notice("Config file is empty" CR );
  } else {
    size_t size = file.size();
    StaticJsonBuffer<188> jsonBuffer;
    if ( size == 0 ) {

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

void publishClusterStatsCallback(){
  Log.notice("Publishing cluster stats via MQTT" CR );
  String cid = String(ESP.getChipId());
  String val = String("{'host':");
  val+= cid;
  val += "}";
  String topic = "/Cluster/timestamp";
  String wsMsg = "{'topic':'";
  wsMsg +=topic;
  wsMsg +="', 'value': ";
  wsMsg +=val;
  wsMsg +="}";
  Log.notice("Sending now MQTT and WS" CR );
  if(wifi_getMode() == WIFI_MODE_AP){
    MQTT_local_publish((unsigned char *)"/Cluster/timestamp", (unsigned char *)val.c_str(), val.length(), 0, 0);
  }else{
    m_mqttClient->publish(topic, val);
  }
  m_websocketServer.textAll(wsMsg.c_str());
  Log.notice("Sent" CR );
}

/////////////////
//Tasks
Task m_publishClusterStatsTask(10000, TASK_FOREVER, &publishClusterStatsCallback);

///////////

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
 Log.notice("on WS event" CR );

 if(type == WS_EVT_CONNECT){
   Log.notice("ws[%s][%u] connect" CR, server->url(), client->id());
   client->printf("Hello Client %u :)", client->id());
   client->ping();
 } else if(type == WS_EVT_DISCONNECT){
   Log.notice("ws[%s][%u] disconnect" CR, server->url(), client->id());
   //Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
 } else if(type == WS_EVT_ERROR){
   Log.warning("ws[%s][%u] error(%u): %s" CR, server->url(), client->id(), *((uint16_t*)arg), (char*)data);
 } else if(type == WS_EVT_PONG){
   //Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
 } else if(type == WS_EVT_DATA){
   Log.notice("ws[%s][%u] data" CR, server->url(), client->id());
   // AwsFrameInfo * info = (AwsFrameInfo*)arg;
   // String msg = "";
   // if(info->final && info->index == 0 && info->len == len){
     //the whole message is in a single frame and we got all of it's data
     // Log.notice("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
   //
   //   if(info->opcode == WS_TEXT){
   //     for(size_t i=0; i < info->len; i++) {
   //       msg += (char) data[i];
   //     }
   //   } else {
   //     char buff[3];
   //     for(size_t i=0; i < info->len; i++) {
   //       sprintf(buff, "%02x ", (uint8_t) data[i]);
   //       msg += buff ;
   //     }
   //   }
   //   Log.notice("%s" CR,msg.c_str());
   //
   //   if(info->opcode == WS_TEXT)
   //     client->text("I got your text message");
   //   else
   //     client->binary("I got your binary message");
   // } else {
   //   // We don't handle multiframe messages
   //   Log.warning("ws - multiframe messages not handled !" CR);
   // }
 }
}


void setup() {
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  Log.notice(CR CR "Starting Chibit ..." CR);

  m_taskScheduler.init();
  Log.notice(CR CR "Initialized scheduler ..." CR);

  m_hostname = wifi_getAPName();
  m_wifiInitialized = wifi_joinOrCreateAP(m_hostname.c_str());

  if(!m_wifiInitialized){
    Log.fatal("Could not start WiFi networking !" CR);
  }else{
    Log.notice("Starting file system ..." CR);
    SPIFFS.begin();

    const int wifiMode = wifi_getMode();
    Log.notice("WiFi mode is now %d" CR, wifiMode);
    switch(wifiMode){
      case WIFI_MODE_AP:
        if (!MDNS.begin("chibit")) {
          Log.fatal("Error setting up MDNS responder!" CR);
        }else{
          Log.notice("mDNS responder started" CR);

          Log.notice("Starting up Web server ..." CR);
          m_websocketServer.onEvent(onWsEvent);
          m_httpServer.addHandler(&m_websocketServer);
          m_httpServer.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
              request->send(200, "text/plain", String(ESP.getFreeHeap()));
            });
          m_httpServer.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
          m_httpServer.begin();
          MDNS.addService("http","tcp",80);

          Log.notice(CR CR "Starting up MQTT broker ..." CR);
          MDNS.addService("mqtt","tcp",DEFAULT_MQTT_PORT);

          MQTT_server_onData(MQTTCallback);
          MQTT_server_start(DEFAULT_MQTT_PORT, 30, 30);

           m_taskScheduler.addTask(m_publishClusterStatsTask);
           m_publishClusterStatsTask.enable();
         }
        break;
      case WIFI_MODE_CLIENT:
      case WIFI_MODE_INFRASTRUCTURE:
        MDNS.begin(m_hostname.c_str());
        Log.notice("Locating MQTT broker ..." CR);
        int n = MDNS.queryService("mqtt", "tcp"); // Send out query for MQTT service
        if (n == 0) {
          Log.fatal( "MQTT broker could not be located" CR);
        } else {
          Log.notice( "Found at least one service" CR);
          m_mqttBrokerIp = MDNS.IP(0);
          m_mqttBrokerPort = MDNS.port(0);
          const String ipAddr = m_mqttBrokerIp.toString();
          Log.notice("Located MQTT broker at %s : %d" CR, ipAddr.c_str(), m_mqttBrokerPort);

          m_mqttClient = new MQTT(m_hostname.c_str(), ipAddr.c_str(), m_mqttBrokerPort);
        }
      break;
    }
  }

}

void loop() {
  if(m_wifiInitialized){
    m_taskScheduler.execute();
  }

}
