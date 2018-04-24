#include "main.h"
#ifdef UNIT_TEST


#include <unity.h>

#include <election.h>

void setup(){
  electionInit(false);
  
  UNITY_BEGIN();
  UNITY_END();

}


void loop() {
    electionUpdate();
}

#endif
