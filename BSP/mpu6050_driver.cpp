//
// Created by HP on 2024/2/1.
//

#include "mpu6050_driver.h"

MPU6050_Driver::MPU6050_Driver(GPIO_TypeDef *SCLPort, uint16_t SCLPin,
                               GPIO_TypeDef *SDAPort, uint16_t SDAPin) {
  iic.Init(SCLPort, SDAPort, SCLPin, SDAPin);
  iic.iic_write_data(MPU6050Addr, PWR_MGMT_1, 0x80);
  HAL_Delay(100);
  iic.iic_write_data(MPU6050Addr, PWR_MGMT_1, 0x02); // PLL，使用X轴陀螺作为参考
  iic.iic_write_data(MPU6050Addr, SMPLRT_DIV, 0x09); // 10ms 采集一次
  iic.iic_write_data(MPU6050Addr, MPU_CONFIG, 0x03);
  iic.iic_write_data(MPU6050Addr, GYRO_CONFIG, 0x18); // gyro scale  ：+-2000°/s
  iic.iic_write_data(MPU6050Addr, ACCEL_CONFIG,
                     0x00); // Accel scale ：+-2g (65536/4=16384 LSB/g)
  HAL_Delay(50);
}

void MPU6050_Driver::Init() {
  iic.iic_write_data(MPU6050Addr, PWR_MGMT_1, 0x80);
  HAL_Delay(100);
  iic.iic_write_data(MPU6050Addr, PWR_MGMT_1, 0x02); // PLL，使用X轴陀螺作为参考
  iic.iic_write_data(MPU6050Addr, SMPLRT_DIV, 0x09); // 10ms 采集一次
  iic.iic_write_data(MPU6050Addr, MPU_CONFIG, 0x03);
  iic.iic_write_data(MPU6050Addr, GYRO_CONFIG, 0x18); // gyro scale  ：+-2000°/s
  iic.iic_write_data(MPU6050Addr, ACCEL_CONFIG,
                     0x08); // Accel scale ：+-4g (65536/4=16384 LSB/g)
  HAL_Delay(50);
}

uint8_t MPU6050_Driver::read_ID() {
  char data = iic.iic_read_data(MPU6050Addr, MPU6050ID);
  return data;
}
void MPU6050_Driver::read_angle(float *roll, float *pitch, float *yaw) {
  short int raw_accx, raw_accy, raw_accz;
  float raw_accxf, raw_accyf, raw_acczf;
  short int raw_gyrox, raw_gyroy, raw_gyroz;

  // Read accelerometer and gyroscope raw data
  raw_accx = readHalfWorld(ACCEL_XOUT_H);
  raw_accy = readHalfWorld(ACCEL_YOUT_H);
  raw_accz = readHalfWorld(ACCEL_ZOUT_H);
  raw_gyrox = readHalfWorld(GYRO_XOUT_H);
  raw_gyroy = readHalfWorld(GYRO_YOUT_H);
  raw_gyroz = readHalfWorld(GYRO_ZOUT_H);

  // Normalize accelerometer values to g (±4g)
  raw_accxf = (float)(raw_accx) / 8192.0;
  raw_accyf = (float)(raw_accy) / 8192.0;
  raw_acczf = (float)(raw_accz) / 8192.0;

  // Normalize gyroscope values to degrees per second (±2000°/s)
  float gyro_x = (float)(raw_gyrox) / 131.0;
  float gyro_y = (float)(raw_gyroy) / 131.0;
  float gyro_z = (float)(raw_gyroz) / 131.0;

  // Calculate roll and pitch from accelerometer data
  *roll = atan2(raw_accyf, raw_acczf) * 180.0 /
          3.14159265358979; // Convert radians to degrees
  *pitch =
      atan2(-raw_accxf, sqrt(raw_accyf * raw_accyf + raw_acczf * raw_acczf)) *
      180.0 / 3.14159265358979;

  // Simple complementary filter for yaw using gyroscope data and accelerometer
  // data (simplified) The yaw calculation is much more complex and typically
  // needs magnetometer data, so here it's skipped
  *yaw = gyro_z; // Assuming no magnetometer, yaw is just the gyroscope's Z-axis
                 // reading

  // You can integrate the gyroscope readings with a complementary filter if
  // needed for more accurate yaw.
  if (0) {
    // 滑窗滤波，过滤异常值
    static float roll_filter[5] = {0};
    static float pitch_filter[5] = {0};
    static float yaw_filter[5] = {0};
    static int filter_index = 0;

    roll_filter[filter_index] = *roll;
    pitch_filter[filter_index] = *pitch;
    yaw_filter[filter_index] = *yaw;
    filter_index++;
    if (filter_index >= 5) {
      filter_index = 0;
    }
    *roll = 0;
    *pitch = 0;
    *yaw = 0;
    for (int i = 0; i < 5; i++) {
      *roll += roll_filter[i];
      *pitch += pitch_filter[i];
      *yaw += yaw_filter[i];
    }
    *roll /= 5;
    *pitch /= 5;
    *yaw /= 5;
  }
}

uint8_t MPU6050_Driver::read(uint8_t addr) {
  return iic.iic_read_data(MPU6050Addr, addr);
}
short int MPU6050_Driver::readHalfWorld(uint8_t addr) {
  uint8_t dataL;
  uint8_t dataH;
  dataH = iic.iic_read_data(MPU6050Addr, addr);
  dataL = iic.iic_read_data(MPU6050Addr, addr + 1);
  return (short int)((dataH << 8) + dataL);
}
void MPU6050_Driver::write(uint8_t addr, uint8_t value) {
  iic.iic_write_data(MPU6050Addr, addr, value);
}