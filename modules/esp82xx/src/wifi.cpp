#include "wifi.h"

#include <Arduino.h>
#include <ArduinoLog.h>

// Required by MQTTbroker library
#include <ESP8266WiFi.h>


bool wifi_joinOrCreateAP(std::string hostname){

#ifndef HARDWIRED_AP
  int n = WiFi.scanNetworks();
  // find the strongest suitable signal
  //  * Preferred : An infrastructure AP
  //  * Otherwise : Any ChibitAP* with the strongest signal
#else
  // For now just connect to a hard coded network
  Log.notice("Connecting to hardwired access point" CR );
  WiFi.hostname(hostname.c_str());
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
  IPAddress myIP = WiFi.localIP();

  Serial.print("AP IP address: ");
  Serial.println(myIP);

  Log.notice("Connected to hardwired access point " AP_SSID CR);
#endif
  return true;
}
