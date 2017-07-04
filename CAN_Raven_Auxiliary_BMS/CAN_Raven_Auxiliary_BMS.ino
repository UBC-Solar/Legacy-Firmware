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

  mpu6050.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  Serial.print(ax); Serial.print(" ");
  Serial.print(ay); Serial.print(" ");
  Serial.print(az); Serial.print(" ");
  Serial.print(gx); Serial.print(" ");
  Serial.print(gy); Serial.print(" ");
  Serial.print(gz); Serial.println("");
}
