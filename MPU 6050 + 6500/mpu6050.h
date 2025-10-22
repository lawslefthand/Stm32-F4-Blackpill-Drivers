/*
 * mpu6050.h
 *
 *  Created on: Aug 16, 2025
 *      Author: lawslefthand
 */

#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_

//Manage MPU6050 internal settings here
//AD0 Addr
#define MPU_6050_ADDR 0x68
//Sample rate setting
#define SMPLRT_DIV_REG 0x19
//Gyro Configuration register
#define GYRO_CONFIG_REG 0x1B
//Accel Configuration register
#define ACCEL_CONFIG_REG 0x1C
//Write 0 PWR_MGMT_1 to wake the sensor up and take readings
#define PWR_MGMT_1_REG 0x6B
//Device ID, Sensor *must* return this
#define WHO_AM_I_REG 0x75

void imu_rcc_config(void);
void imu_gpio_config(void);
void imu_i2c_config(void);
void mpu_start(void);


void imu_i2c_write(uint8_t addr, uint8_t value);
uint8_t imu_i2c_read(uint8_t reg_addr);

void mpu_init(void);
void mpu_accel_read(void);
void mpu_gyro_read(void);

int16_t mpu_accel_read_x(void);
int16_t mpu_accel_read_y(void);
int16_t mpu_accel_read_z(void);

int16_t mpu_gyro_read_x(void);
int16_t mpu_gyro_read_y(void);
int16_t mpu_gyro_read_z(void);


#endif /* INC_MPU6050_H_ */
