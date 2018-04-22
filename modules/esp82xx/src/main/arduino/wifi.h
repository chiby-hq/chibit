#ifndef __WIFI_H

#define __WIFI_H

#include <Arduino.h>

#ifdef HARDWIRED_AP
#include "secret_hardwiredAP.h"
#endif


#define WIFI_MODE_DISCONNECTED 0
#define WIFI_MODE_INFRASTRUCTURE 1
#define WIFI_MODE_AP 2
#define WIFI_MODE_CLIENT 4

typedef struct {
  const char* ssid;
  const char* password;
} wifiAccessPoint;

// Predefined access points, preferred to
const wifiAccessPoint ACCESS_POINTS_LIST[] {
    {"CERN", ""},
    {"codezlascience_public", ""},
    {"ChibitAP", ""}
};

bool wifi_joinOrCreateAP(const char*);

int wifi_getMode();

const String wifi_getAPName();
#endif
