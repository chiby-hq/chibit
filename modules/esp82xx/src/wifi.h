#ifndef __WIFI_H

#define __WIFI_H

#include <Arduino.h>

#ifdef HARDWIRED_AP
#include "secret_hardwiredAP.h"
#endif

bool wifi_joinOrCreateAP(std::string);

#endif
