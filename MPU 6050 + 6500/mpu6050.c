/*
 * mpu6050.c
 *
 *  Created on: Aug 16, 2025
 *      Author: lawslefthand
 */

#include "stm32f4xx.h"
#include <stdio.h>
#include "mpu6050.h"

int16_t Accel_X_RAW;
int16_t Accel_Y_RAW;
int16_t Accel_Z_RAW;

int16_t Gyro_X_RAW;
int16_t Gyro_Y_RAW;
int16_t Gyro_Z_RAW;

float Ax, Ay, Az, Gx, Gy, Gz;


void imu_rcc_config(void) {
	//GPIOB clock + I2C1 Clk
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
}

void imu_gpio_config(void) {
	//PB8 (SCL) and PB9 (SDA) for I2C1
	GPIOB->MODER &= ~(GPIO_MODER_MODER8_0);
	GPIOB->MODER |= GPIO_MODER_MODER8_1;
	GPIOB->MODER &= ~(GPIO_MODER_MODER9_0);
	GPIOB->MODER |= GPIO_MODER_MODER9_1;

	GPIOB->OTYPER |= GPIO_OTYPER_OT_8;
	GPIOB->OTYPER |= GPIO_OTYPER_OT_9;

	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR8_1);
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR8_0;
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR9_1);
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR9_0;

	GPIOB->AFR[1] &= ~(0xF << 0);
	GPIOB->AFR[1] |= (4 << 0);

	GPIOB->AFR[1] &= ~(0xF << 4);
	GPIOB->AFR[1] |= (4 << 4);
}

void imu_i2c_config(void) {

	I2C1->CR1 &= ~I2C_CR1_PE;
	while (I2C1->CR1 & I2C_CR1_PE)
		;

	// I2C (Standard mode, 100Khz)

	I2C1->CR2 = (42 & 0x3F);

	// Tscl = CCR * Tpclk1 * 2
	// For 100kHz: 10µs = CCR * (1/42MHz) * 2
	// CCR = 10µs / (2 * (1/42MHz)) = 210
	I2C1->CCR = 210;
	I2C1->TRISE = 43;
	I2C1->CR1 |= I2C_CR1_PE;
}

void imu_i2c_write(uint8_t addr, uint8_t value) {
	while (I2C1->SR2 & I2C_SR2_BUSY);
	I2C1->CR1 |= I2C_CR1_START;

	while (!(I2C1->SR1 & I2C_SR1_SB));
	I2C1->DR = MPU_6050_ADDR << 1;

	while (!(I2C1->SR1 & I2C_SR1_ADDR));
	(void) I2C1->SR2;

	while (!(I2C1->SR1 & I2C_SR1_TXE));
	I2C1->DR = addr;

	while (!(I2C1->SR1 & I2C_SR1_TXE));
	I2C1->DR = value;

	while (!(I2C1->SR1 & I2C_SR1_BTF));
	I2C1->CR1 |= I2C_CR1_STOP;
}

uint8_t imu_i2c_read(uint8_t reg_addr) {
    uint8_t value;

    while (I2C1->SR2 & I2C_SR2_BUSY);
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));

    I2C1->DR = MPU_6050_ADDR << 1;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR1; (void)I2C1->SR2;

    while (!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = reg_addr;
    while (!(I2C1->SR1 & I2C_SR1_BTF));

    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));

    I2C1->DR = (MPU_6050_ADDR << 1) | 0x01;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
   (void)I2C1->SR1; (void)I2C1->SR2;

    I2C1->CR1 &= ~I2C_CR1_ACK;
    I2C1->CR1 |= I2C_CR1_STOP;

    while (!(I2C1->SR1 & I2C_SR1_RXNE));
    value = I2C1->DR;

    while (I2C1->CR1 & I2C_CR1_STOP);
    I2C1->CR1 |= I2C_CR1_ACK;

    return value;
}


void mpu_init(void) {
	uint8_t check;

	check = imu_i2c_read(WHO_AM_I_REG);
	printf("ID is %u", check);

	if (check == 0x68) {

		printf("Correct sensor ID\n");
		imu_i2c_write(PWR_MGMT_1_REG, 0);
		imu_i2c_write(SMPLRT_DIV_REG, 0x07);
		imu_i2c_write(GYRO_CONFIG_REG, 0x00);
		imu_i2c_write(ACCEL_CONFIG_REG, 0x00);

	} else {
		printf("Invalid sensor ID\n");
	}

}

void mpu_accel_read(void) {
	uint8_t Rec_Data[6];

	Rec_Data[0] = imu_i2c_read(0x3B);
	Rec_Data[1] = imu_i2c_read(0x3C);
	Rec_Data[2] = imu_i2c_read(0x3D);
	Rec_Data[3] = imu_i2c_read(0x3E);
	Rec_Data[4] = imu_i2c_read(0x3F);
	Rec_Data[5] = imu_i2c_read(0x40);

	Accel_X_RAW = (uint16_t) (Rec_Data[0] << 8 | Rec_Data[1]);
	Accel_Y_RAW = (uint16_t) (Rec_Data[2] << 8 | Rec_Data[3]);
	Accel_Z_RAW = (uint16_t) (Rec_Data[4] << 8 | Rec_Data[5]);

	Ax = (float) Accel_X_RAW / 16384.0;
	Ay = (float) Accel_Y_RAW / 16384.0;
	Az = (float) Accel_Z_RAW / 16384.0;

	printf("Accel: X=%.2f g, Y=%.2f g, Z=%.2f g\r\n", Ax, Ay, Az);

}

int16_t mpu_accel_read_x(void) {

	uint8_t Rec_Data[6];

	Rec_Data[0] = imu_i2c_read(0x3B);
	Rec_Data[1] = imu_i2c_read(0x3C);
	Rec_Data[2] = imu_i2c_read(0x3D);
	Rec_Data[3] = imu_i2c_read(0x3E);
	Rec_Data[4] = imu_i2c_read(0x3F);
	Rec_Data[5] = imu_i2c_read(0x40);

	Accel_X_RAW = (uint16_t) (Rec_Data[0] << 8 | Rec_Data[1]);
	Accel_Y_RAW = (uint16_t) (Rec_Data[2] << 8 | Rec_Data[3]);
	Accel_Z_RAW = (uint16_t) (Rec_Data[4] << 8 | Rec_Data[5]);

	Ax = (float) Accel_X_RAW / 16384.0;

	//printf("Accel: X=%.2f g\n", Ax);

	return Ax;

}

int16_t mpu_accel_read_y(void) {

	uint8_t Rec_Data[6];

	Rec_Data[0] = imu_i2c_read(0x3B);
	Rec_Data[1] = imu_i2c_read(0x3C);
	Rec_Data[2] = imu_i2c_read(0x3D);
	Rec_Data[3] = imu_i2c_read(0x3E);
	Rec_Data[4] = imu_i2c_read(0x3F);
	Rec_Data[5] = imu_i2c_read(0x40);

	Accel_X_RAW = (uint16_t) (Rec_Data[0] << 8 | Rec_Data[1]);
	Accel_Y_RAW = (uint16_t) (Rec_Data[2] << 8 | Rec_Data[3]);
	Accel_Z_RAW = (uint16_t) (Rec_Data[4] << 8 | Rec_Data[5]);

	Ay = (float) Accel_Y_RAW / 16384.0;

	//printf("Accel: Y=%.2f g\n", Ay);

	return Ay;
}

int16_t mpu_accel_read_z(void) {

	uint8_t Rec_Data[6];

	Rec_Data[0] = imu_i2c_read(0x3B);
	Rec_Data[1] = imu_i2c_read(0x3C);
	Rec_Data[2] = imu_i2c_read(0x3D);
	Rec_Data[3] = imu_i2c_read(0x3E);
	Rec_Data[4] = imu_i2c_read(0x3F);
	Rec_Data[5] = imu_i2c_read(0x40);

	Accel_X_RAW = (uint16_t) (Rec_Data[0] << 8 | Rec_Data[1]);
	Accel_Y_RAW = (uint16_t) (Rec_Data[2] << 8 | Rec_Data[3]);
	Accel_Z_RAW = (uint16_t) (Rec_Data[4] << 8 | Rec_Data[5]);

	Az = (float) Accel_Z_RAW / 16384.0;


	//printf("Accel: Z=%.2f g\n", Az);

	return Az;
}


void mpu_gyro_read(void) {
	uint8_t Rec_Data[6];

	Rec_Data[0] = imu_i2c_read(0x43);
	Rec_Data[1] = imu_i2c_read(0x44);
	Rec_Data[2] = imu_i2c_read(0x45);
	Rec_Data[3] = imu_i2c_read(0x46);
	Rec_Data[4] = imu_i2c_read(0x47);
	Rec_Data[5] = imu_i2c_read(0x48);

	Gyro_X_RAW = (uint16_t) (Rec_Data[0] << 8 | Rec_Data[1]);
	Gyro_Y_RAW = (uint16_t) (Rec_Data[2] << 8 | Rec_Data[3]);
	Gyro_Z_RAW = (uint16_t) (Rec_Data[4] << 8 | Rec_Data[5]);

	Gx = (float) Gyro_X_RAW / 131.0;
	Gy = (float) Gyro_Y_RAW / 131.0;
	Gz = (float) Gyro_Z_RAW / 131.0;

	printf("Gyro: X=%.2f dps, Y=%.2f, Z=%.2f dps\n", Gx, Gy, Gz);

}

int16_t mpu_gyro_read_x(void) {
	uint8_t Rec_Data[6];

	Rec_Data[0] = imu_i2c_read(0x43);
	Rec_Data[1] = imu_i2c_read(0x44);
	Rec_Data[2] = imu_i2c_read(0x45);
	Rec_Data[3] = imu_i2c_read(0x46);
	Rec_Data[4] = imu_i2c_read(0x47);
	Rec_Data[5] = imu_i2c_read(0x48);

	Gyro_X_RAW = (uint16_t) (Rec_Data[0] << 8 | Rec_Data[1]);
    Gyro_Y_RAW = (uint16_t) (Rec_Data[2] << 8 | Rec_Data[3]);
    Gyro_Z_RAW = (uint16_t) (Rec_Data[4] << 8 | Rec_Data[5]);

	Gx = (float) Gyro_X_RAW / 131.0;

	//printf("Gyro: X=%.2f dps\n", Gx);

	return Gx;

}

int16_t mpu_gyro_read_y(void) {
	uint8_t Rec_Data[6];

	Rec_Data[0] = imu_i2c_read(0x43);
	Rec_Data[1] = imu_i2c_read(0x44);
	Rec_Data[2] = imu_i2c_read(0x45);
	Rec_Data[3] = imu_i2c_read(0x46);
	Rec_Data[4] = imu_i2c_read(0x47);
	Rec_Data[5] = imu_i2c_read(0x48);

	Gyro_X_RAW = (uint16_t) (Rec_Data[0] << 8 | Rec_Data[1]);
    Gyro_Y_RAW = (uint16_t) (Rec_Data[2] << 8 | Rec_Data[3]);
    Gyro_Z_RAW = (uint16_t) (Rec_Data[4] << 8 | Rec_Data[5]);

	Gy = (float) Gyro_Y_RAW / 131.0;


	//printf("Gyro: Y=%.2f dps\n", Gy);

	return Gy;

}

int16_t mpu_gyro_read_z(void) {
	uint8_t Rec_Data[6];

	Rec_Data[0] = imu_i2c_read(0x43);
	Rec_Data[1] = imu_i2c_read(0x44);
	Rec_Data[2] = imu_i2c_read(0x45);
	Rec_Data[3] = imu_i2c_read(0x46);
	Rec_Data[4] = imu_i2c_read(0x47);
	Rec_Data[5] = imu_i2c_read(0x48);

	Gyro_X_RAW = (uint16_t) (Rec_Data[0] << 8 | Rec_Data[1]);
    Gyro_Y_RAW = (uint16_t) (Rec_Data[2] << 8 | Rec_Data[3]);
    Gyro_Z_RAW = (uint16_t) (Rec_Data[4] << 8 | Rec_Data[5]);

    Gz = (float) Gyro_Z_RAW / 131.0;

	//printf("Gyro: Z=%.2f dps\n", Gz);

	return Gz;

}

void mpu_start(void)
{
   imu_rcc_config();
   imu_gpio_config();
   imu_i2c_config();
}
