#include "analog_bend.h"
#include <ArduinoLog.h>

// variables:
unsigned int m_bendSensorValue = 0;         // the sensor value
unsigned int m_bendSensorMin = 1023;        // minimum sensor value
unsigned int m_bendSensorMax = 0;           // maximum sensor value

AnalogSmooth m_bendSensorAverage = AnalogSmooth(15); //averaging filter
unsigned char m_bendSensorOutputValue = 0;        //the scaled & smoothed result: 0 - 255

unsigned char m_ledStatus = HIGH;        //for doing the calibration blink, 1 = off

bool analog_bend_calibrateSensor(unsigned int &min, unsigned int &max){
  //calibration
  Log.notice("Calibrating..." CR );
  unsigned int now = millis();
  while ( (millis() - now) < 7000) { //calibrate
    m_bendSensorValue = analogRead(BEND_SENSOR_PIN);

    // record the maximum sensor value
    if (m_bendSensorValue > m_bendSensorMax) {
      m_bendSensorMax = m_bendSensorValue;
    }

    // record the minimum sensor value
    if (m_bendSensorValue < m_bendSensorMin) {
      m_bendSensorMin = m_bendSensorValue;
    }
    m_ledStatus = !m_ledStatus;
    delay(50);
    digitalWrite(LED_PIN, m_ledStatus);
  }
  m_ledStatus = HIGH;
  digitalWrite(LED_PIN, m_ledStatus); //off!

  min = m_bendSensorMin;
  max = m_bendSensorMax;
}

unsigned char analog_bend_getLastKnownAdcRead(){
  return m_bendSensorOutputValue;
}

unsigned char analog_bend_readAdc(){
  // read the analog in value:
    m_bendSensorValue = analogRead(BEND_SENSOR_PIN);

    // apply the calibration to the sensor reading
    m_bendSensorValue = map(m_bendSensorValue, m_bendSensorMin, m_bendSensorMax, 0, 255);

    // in case the sensor value is outside the range seen during calibration
    m_bendSensorValue = constrain(m_bendSensorValue, 0, 255);
    m_bendSensorOutputValue = m_bendSensorAverage.smooth(m_bendSensorValue);

    return m_bendSensorOutputValue;
}

void analog_bend_setMinMax(unsigned int min, unsigned int max){
  m_bendSensorMin = min;
  m_bendSensorMax = max;
}
