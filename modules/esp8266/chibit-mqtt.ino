
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

#include <easyMesh.h>
#include <Ticker.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#include "FS.h"

#include "uMQTTBroker.h"

#ifndef WIFI_SSID
#include "secrets.h"
#endif

char ssid[] = WIFI_SSID;    // your network SSID (name)
char pass[] = WIFI_PASSWORD; // your network password

void sendMessage() ; // Prototype

ESP8266WebServer server ( 80 );

easyMesh  mesh;
Ticker meshUpdateTimer;

void meshUpdate(){  
  mesh.update();  //update mesh parameters 
}


void setup() {
  
  Serial.begin(115200);
  

  
  Serial.println();
  Serial.println();

  randomSeed(analogRead(0));
  
  SPIFFS.begin();
  Serial.println("Initialized file system");
  
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if ( MDNS.begin ( "esp8266" ) ) {
    Serial.println ( "MDNS responder started" );
  }
  
  server.on ( "/", []() {
    server.send ( 200, "text/plain", "Hello world" );
  } );
  server.serveStatic("/paho-mqtt.js", SPIFFS, "/paho-mqtt.js");

  server.onNotFound ( handleNotFound );

  server.begin();
  Serial.println ( "HTTP server started" );
 
  //call meshUpdate function every 3 sec 
 // meshUpdateTimer.attach(3,meshUpdate); //delay depends on how frequently nodes are moving   

 // mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
 // mesh.setReceiveCallback( &receivedCallback );
 // mesh.setNewConnectionCallback( &newConnectionCallback );

}

void loop() {
    server.handleClient();
  //   String msg = "Hello from node ";
  //  msg += mesh.getChipId();
  //  mesh.sendBroadcast( msg );
  
}


/** ****************************************
 * MQTT related routines
 */
void MQTTDataCallback(uint32_t *client /* we can ignore this */, const char* topic, uint32_t topic_len, const char *data, uint32_t lengh) {
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

/** ****************************************
 *  Web Server related routines
 */
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

/** ****************************************
 *  Mesh-related functions
 */

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %d msg=%s\n", from, msg.c_str());
}

void newConnectionCallback( bool adopt ) {
  Serial.printf("startHere: New Connection, adopt=%d\n", adopt);
}


/** ****************************************
 *  Misc utility functions
 */
void setupAPandMQTTBroker(){
  // Wait for a random amount of time
  // Start an AP
     // If successful, start up a MQTT Broker
     MQTT_server_onData(MQTTDataCallback);
     MQTT_server_start(1883, 30, 30);


}

