#include "wifi.h"
#include "globals.h"

#include <Arduino.h>
#include <ArduinoLog.h>

// Required by MQTTbroker library
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti wifiMulti;
const int MAXCOUNT=10;

IPAddress m_apIP(192, 168, 4, 1);

const char* m_apSSID;
const char* m_apPassword;

int m_wifiMode = WIFI_MODE_DISCONNECTED;



bool _wifi_connectWithTimeout(const char* ssid, const char* password, int timeout){
  WiFi.begin(ssid, password);
  int count = 0;
  while (count <= MAXCOUNT && WiFi.status() != WL_CONNECTED) {
    delay(timeout / MAXCOUNT);
    count++;
  }
  if( count >=MAXCOUNT && WiFi.status() != WL_CONNECTED){
     Log.warning("Could not connect to access point %s" CR, ssid);
     return false;
  }

  return true;
}

int wifi_getMode(){
  return m_wifiMode;
}

const String wifi_getAPName(){
  return String(CHIBIT_AP_PREFIX+String(ESP.getChipId()));
}

bool wifi_joinOrCreateAP(const char* hostname){
  WiFi.hostname(hostname);
  bool connected = false;

#ifndef HARDWIRED_AP
  // Start in station mode
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();

  // Find the strongest suitable signal
  //  * (1/3) Preferred : An infrastructure AP
  Log.notice("Scanning known infrastructure access points" CR );
  for (int i = 0; i < n && (!connected); ++i) {
    for(uint8_t j = 0; j < sizeof(ACCESS_POINTS_LIST)/sizeof(wifiAccessPoint); ++j) {
      if(ACCESS_POINTS_LIST[j].ssid == WiFi.SSID(i).c_str()){
        Log.notice("Trying to connect to known infrastructure access point %s" CR, ACCESS_POINTS_LIST[j].ssid);
        connected = _wifi_connectWithTimeout(ACCESS_POINTS_LIST[j].ssid, ACCESS_POINTS_LIST[j].password, WIFI_TIMEOUT_MS);
        if(connected){
          Log.notice("Successful connection to Infrastructure AP %s" CR, ACCESS_POINTS_LIST[j].ssid);
          m_wifiMode = WIFI_MODE_INFRASTRUCTURE;
        }else{
          Log.notice("No known infrastructure access point found, moving on..." CR);
        }
      }
    }
  }

  //  * (2/3) Otherwise : Any ChibitAP_* with signal strength (RSSI) >= -40 dB
  if(!connected){
    Log.notice("Trying to connect to any nearby Chibit ad-hoc access point" CR );
    for (int i = 0; i < n && (!connected); ++i) {
      //Log.notice("SSID %s RRSI %d" , WiFi.SSID(i).c_str(), WiFi.RSSI(i) );
      if(WiFi.SSID(i).startsWith(CHIBIT_AP_PREFIX) && WiFi.RSSI(i) >= -40){
        // Connect to the ad-hoc Chibit Access Point - the password is the SSID + "passwd"
        String apPassword = WiFi.SSID(i)+"passwd";
        Log.notice("Trying to connect to ad-hoc Chibit access point %s (%d) with password %s" CR, WiFi.SSID(i).c_str(), WiFi.RSSI(i), apPassword.c_str() );
        connected = _wifi_connectWithTimeout(WiFi.SSID(i).c_str(), apPassword.c_str(), WIFI_TIMEOUT_MS);
        if(connected){
          Log.notice("Successful connection to ad-hoc Chibit access point %s" CR, WiFi.SSID(i).c_str());
          m_wifiMode = WIFI_MODE_CLIENT;
        }else{
          Log.notice("No nearby Chibit ad-hoc access point found, starting my own..." CR);
        }
      }
    }
  }
  // * (3/3) if not, start its own ChibitAP_<chipId> and wait for connections
  if(!connected){
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(m_apIP, m_apIP, IPAddress(255, 255, 255, 0));
    m_apSSID = wifi_getAPName().c_str();

    String apPassword = wifi_getAPName()+"passwd";
    m_apPassword = apPassword.c_str();
    WiFi.softAP(m_apSSID, m_apPassword);

    Log.notice("Starting ad-hoc access point '%s' : '%s'" CR, m_apSSID, m_apPassword);

    String softAPIP = WiFi.softAPIP().toString();
    Log.notice("AP IP address: %s" CR, softAPIP.c_str());
    m_wifiMode = WIFI_MODE_AP;
    connected = true;

  }
#else
  // For now just connect to a hard coded network
  Log.notice("Connecting to hardwired access point" CR );
  connected = _wifi_connectWithTimeout(AP_SSID, AP_PASS, WIFI_TIMEOUT_MS);

  if(connected){
    IPAddress myIP = WiFi.localIP();
    Log.notice("Connected to hardwired access point %s" CR,  AP_SSID);
    Serial.print("Local IP address: ");
	  Serial.println(myIP);
    m_wifiMode = WIFI_MODE_INFRASTRUCTURE;
  }
#endif
  return connected;
}
