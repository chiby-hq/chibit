#ifndef __ANALOG_BEND_H

#define __ANALOG_BEND_H
#include "globals.h"

#include <AnalogSmooth.h>


const int BEND_SENSOR_PIN = A0;  // Analog input pin that the bend sensor is attached to


bool analog_bend_calibrateSensor(unsigned int &min, unsigned int &max);

unsigned char analog_bend_getLastKnownAdcRead();

unsigned char analog_bend_readAdc();

void analog_bend_setMinMax(unsigned int min, unsigned int max);

#endif
