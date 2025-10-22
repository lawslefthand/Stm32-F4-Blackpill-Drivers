/*
 * qmc5883p.c
 *
 *  Created on: Oct 22, 2025
 *      Author: danba
 */

#include <math.h>
#include <stdio.h>
#include <stm32f4xx.h>
#include <qmc5883p.h>


float mag_x, mag_y, mag_z = 0;

void qmc_rcc_config(void) {
	//GPIOB clock + I2C1 Clk
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
}

void qmc_gpio_config(void) {
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

void qmc_i2c_config(void) {

	I2C1->CR1 &= ~I2C_CR1_PE;
	while (I2C1->CR1 & I2C_CR1_PE);

	// I2C (Standard mode, 100Khz)

	I2C1->CR2 = (42 & 0x3F);

	// Tscl = CCR * Tpclk1 * 2
	// For 100kHz: 10µs = CCR * (1/42MHz) * 2
	// CCR = 10µs / (2 * (1/42MHz)) = 210
	I2C1->CCR = 210;
	I2C1->TRISE = 43;
	I2C1->CR1 |= I2C_CR1_PE;
}

void qmc_i2c_write(uint8_t addr, uint8_t value) {
	while (I2C1->SR2 & I2C_SR2_BUSY)
		;
	I2C1->CR1 |= I2C_CR1_START;

	while (!(I2C1->SR1 & I2C_SR1_SB))
		;
	I2C1->DR = QMC_ADDR << 1;

	while (!(I2C1->SR1 & I2C_SR1_ADDR))
		;
	(void) I2C1->SR2;

	while (!(I2C1->SR1 & I2C_SR1_TXE))
		;
	I2C1->DR = addr;

	while (!(I2C1->SR1 & I2C_SR1_TXE))
		;
	I2C1->DR = value;

	while (!(I2C1->SR1 & I2C_SR1_BTF))
		;
	I2C1->CR1 |= I2C_CR1_STOP;
}

uint8_t qmc_i2c_read(uint8_t reg_addr) {
	uint8_t value;

	while (I2C1->SR2 & I2C_SR2_BUSY)
		;
	I2C1->CR1 |= I2C_CR1_START;
	while (!(I2C1->SR1 & I2C_SR1_SB))
		;

	I2C1->DR = QMC_ADDR << 1;
	while (!(I2C1->SR1 & I2C_SR1_ADDR))
		;
	(void) I2C1->SR1;
	(void) I2C1->SR2;

	while (!(I2C1->SR1 & I2C_SR1_TXE))
		;
	I2C1->DR = reg_addr;
	while (!(I2C1->SR1 & I2C_SR1_BTF))
		;

	I2C1->CR1 |= I2C_CR1_START;
	while (!(I2C1->SR1 & I2C_SR1_SB))
		;

	I2C1->DR = (QMC_ADDR << 1) | 0x01;
	while (!(I2C1->SR1 & I2C_SR1_ADDR))
		;
	(void) I2C1->SR1;
	(void) I2C1->SR2;

	I2C1->CR1 &= ~I2C_CR1_ACK;
	I2C1->CR1 |= I2C_CR1_STOP;

	while (!(I2C1->SR1 & I2C_SR1_RXNE))
		;
	value = I2C1->DR;

	while (I2C1->CR1 & I2C_CR1_STOP)
		;
	I2C1->CR1 |= I2C_CR1_ACK;

	return value;
}


void qmc_mag_read(void) {
	uint8_t Rec_Data[6];
	float angle = 0;

	Rec_Data[0] = qmc_i2c_read(0x02); // Read XOUT MSB
	Rec_Data[1] = qmc_i2c_read(0x01); // Read XOUT LSB
	Rec_Data[2] = qmc_i2c_read(0x04); // Read YOUT MSB
	Rec_Data[3] = qmc_i2c_read(0x03); // Read YOUT LSB
	Rec_Data[4] = qmc_i2c_read(0x06); // Read ZOUT MSB
	Rec_Data[5] = qmc_i2c_read(0x05); // Read ZOUT LSB

	mag_x = (int16_t) (Rec_Data[0] << 8 | Rec_Data[1]);
	mag_y = (int16_t) (Rec_Data[2] << 8 | Rec_Data[3]);
	mag_z = (int16_t) (Rec_Data[4] << 8 | Rec_Data[5]);



	//printf("mag: X=%.2f , Y=%.2f , Z=%.2f \r\n", mag_x, mag_y, mag_z);

	angle = atan2(mag_y, mag_x);

	 angle = angle * (180.0f / 3.14159f);

	if (angle <0)
	{
		angle+=360.0f;
	}

	printf("Heading is %f\n",angle);

}



void qmc_init(void) {
	uint8_t check;

	// Read Chip ID
	check = qmc_i2c_read(0x00);
	printf("ID is 0x%X\r\n", check);

	// Chip id may vary with clones. Please check datasheet and verify on own caution.
	if (check == 0x80) {
		printf("Correct sensor ID\r\n");

		/*
		 * Based on Datasheet "Normal Mode Setup Example"
		 */

		// 0x08 to Register 0BH
		// Field Range to 8 Gauss [cite: 403, 585]
		qmc_i2c_write(0x0B, 0x08);

		// 0xCD to Register 0AH
		// Normal Mode, ODR=200Hz, OSR1=8, OSR2=8
		qmc_i2c_write(0x0A, 0xCD);

	} else {
		printf("Invalid sensor ID\r\n");
	}
}





