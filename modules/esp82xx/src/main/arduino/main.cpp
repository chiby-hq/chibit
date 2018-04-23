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

#include "analog_bend.h"

//
//////////////////////

#define CONFIG_FILE "/config.json"

AsyncWebServer m_httpServer(80);
AsyncWebSocket m_websocketServer("/ws");

String m_hostname;
IPAddress m_mqttBrokerIp;
uint16_t m_mqttBrokerPort = DEFAULT_MQTT_PORT;

MQTT* m_mqttClient;

void forceCalibration(){
  unsigned int min;
  unsigned int max;

  Log.notice("Starting forced calibration" CR );

  analog_bend_calibrateSensor(min, max);
  // Save the result
  StaticJsonBuffer<58> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["min"] = min;
  root["max"] = max;

  File file = SPIFFS.open(CONFIG_FILE, "w");
  if (root.printTo(file) == 0) {
    Log.error("Failed to write to configuration file." CR);
  }else{
    Log.notice("JSON configuration file creation successful." CR);
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();
}

void loadConfiguration() {
  File file = SPIFFS.open(CONFIG_FILE, "r");
  if (!file){
    // No config file exists, create one
     Log.notice("Config file is empty" CR );
     file.close();
     forceCalibration();
  } else {
    size_t size = file.size();
    StaticJsonBuffer<58> jsonBuffer;
    if ( size == 0 ) {
      // The file is empty, force a calibration round
      file.close();
      forceCalibration();
    } else {
      std::unique_ptr<char[]> buf (new char[size]);
      file.readBytes(buf.get(), size);
      JsonObject& root = jsonBuffer.parseObject(buf.get());
      if (!root.success()) {
        //  cannot read JSON file
        file.close();
        forceCalibration();
      } else {
        // Configuration loaded - setup calibration min/max
        analog_bend_setMinMax(root["min"], root["max"]);
        Log.notice("JSON configuration file loading successful (min %d, max %d)." CR, root["min"], root["max"]);
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


void publishADCReadingCallback(){
  Log.notice("Publishing ADC reading via MQTT" CR );
  String topic = "/Cluster/adc/reading";
  String val = "123456";
  if(wifi_getMode() == WIFI_MODE_AP){
    MQTT_local_publish((unsigned char *)topic.c_str(),(unsigned char *)val.c_str(), val.length(), 0, 0);
  }else{
    m_mqttClient->publish(topic, val);
  }
}

/////////////////
//Tasks
Task m_publishClusterStatsTask(10000, TASK_FOREVER, &publishClusterStatsCallback);
Task m_publishADCReadingTask(5000, TASK_FOREVER, &publishADCReadingCallback);

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
 }
}


void setup() {
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  Log.notice(CR CR "Starting Chibit ..." CR);

  Log.notice("Starting file system ..." CR);
  SPIFFS.begin();
  Log.notice("Loading initial configuration ..." CR);
  loadConfiguration();

  m_taskScheduler.init();
  Log.notice(CR "Initialized scheduler ..." CR);

  m_hostname = wifi_getAPName();
  m_wifiInitialized = wifi_joinOrCreateAP(m_hostname.c_str());

  if(!m_wifiInitialized){
    Log.fatal("Could not start WiFi networking !" CR);
    // flatline the built-in LED
    digitalWrite(LED_PIN, LOW);
  }else{
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
          // flatline the built-in LED
          digitalWrite(LED_PIN, LOW);
        } else {
          Log.notice( "Found at least one service" CR);
          m_mqttBrokerIp = MDNS.IP(0);
          m_mqttBrokerPort = MDNS.port(0);
          const String ipAddr = m_mqttBrokerIp.toString();
          Log.notice("Located MQTT broker at %s : %d" CR, ipAddr.c_str(), m_mqttBrokerPort);

          m_mqttClient = new MQTT(m_hostname.c_str(), ipAddr.c_str(), m_mqttBrokerPort);

          m_taskScheduler.addTask(m_publishADCReadingTask);
          m_publishADCReadingTask.enable();
        }
      break;
    }
  }

}

void loop() {
  if (digitalRead(FLASH_BUTTON_PIN) == LOW) { //flash button pressed?
    forceCalibration();
  }

  if(m_wifiInitialized){
    m_taskScheduler.execute();
  }

}
