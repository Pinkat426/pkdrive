//
// Created by HP on 2024/2/1.
//

#ifndef AURORA_ST_MPU6050_DRIVER_H
#define AURORA_ST_MPU6050_DRIVER_H

#include "IIC.h"
#include "common_inc.h"

// mpu6050寄存器
#define MPU6050Addr 0xD0
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40

#define GYRO_XOUT_H 0x43
#define GYRO_XOUT_L 0x44
#define GYRO_YOUT_H 0x45
#define GYRO_YOUT_L 0x46
#define GYRO_ZOUT_H 0x47
#define GYRO_ZOUT_L 0x48

#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C
#define SMPLRT_DIV 0x19
#define MPU_CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C

#define MPU6050ID 0x75

class MPU6050_Driver {

public:
  MPU6050_Driver(GPIO_TypeDef *SCLPort, uint16_t SCLPin, GPIO_TypeDef *SDAPort,
                 uint16_t SDAPin);

  void Init();
  uint8_t read_ID();
  void read_angle(float *roll, float *pitch, float *yaw);

  uint8_t read(uint8_t addr);
  short int readHalfWorld(uint8_t addr);
  void write(uint8_t addr, uint8_t value);

private:
private:
  IIC iic;
};

#endif // AURORA_ST_MPU6050_DRIVER_H
