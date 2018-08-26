#include <Arduino.h>
#include <Wire.h>
#include <Homie.h>
#include "gy-521.h"


HomieNode gyroNode("gyro", "gyro");


void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware("bare-minimum", "1.0.0"); // The underscore is not a typo! See Magic bytes
  Homie.setup();

  gyroNode.advertise("GyX");
  gyroNode.advertise("GyY");
  gyroNode.advertise("GyZ");
  gyroNode.advertise("Temp");

  mpu6050Begin(MPU_addr);

}

void loop() {
  updateGy521Sensor(&gyroNode);
  delay(500);  // Wait 5 seconds and scan again


  Homie.loop();

}
