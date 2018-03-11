#ifndef UNIT_TEST
#include "main.h"

#include "election.h"

void setup() {
  electionInit(false);
}

void loop() {
  electionUpdate();

}

#endif
