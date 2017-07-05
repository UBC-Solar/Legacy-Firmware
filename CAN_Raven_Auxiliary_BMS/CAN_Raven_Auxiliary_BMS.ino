#include <FlexCAN.h>
#include <kinetis_flexcan.h>
#include <ubcsolar_can_ids.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"

MPU6050 mpu6050;
int16_t ax, ay, az, gx, gy, gz;

void setup() {

  Wire.begin();
  Serial.begin(115200);
  mpu6050.initialize();
}

void loop() {

}
