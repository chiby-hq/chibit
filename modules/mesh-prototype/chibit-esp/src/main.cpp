#ifndef UNIT_TEST
#include "main.h"
#include <ArduinoLog.h>

#include "election.h"

void setup() {
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  electionInit(false);
}

void loop() {
  electionUpdate();

}

#endif
