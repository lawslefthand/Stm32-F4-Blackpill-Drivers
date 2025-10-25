/*
 * bmp280.c
 *
 *  Created on: Apr 2, 2025
 *      Author: danba
 */



#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include "stm32f4xx.h"
#include "usart.h"

// I2C device address for BMP280 (SDO connected to GND = 0x76, SDO connected to VDDIO = 0x77)
#define BMP280_I2C_ADDR 0x76

void delay_1s(void) {
    for (uint32_t i = 0; i < 1000000; i++) {
        __NOP();
    }
}

void bmp_rcc_config(void) {
    // Enable GPIOB clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // Enable I2C1 clock
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
}

void bmp_gpio_config(void) {
    // Configure PB8 (SCL) and PB9 (SDA) for I2C1

    // Set mode to alternate function
    GPIOB->MODER &= ~(GPIO_MODER_MODER8_0);
    GPIOB->MODER |= GPIO_MODER_MODER8_1;
    GPIOB->MODER &= ~(GPIO_MODER_MODER9_0);
    GPIOB->MODER |= GPIO_MODER_MODER9_1;

    // Set to open-drain
    GPIOB->OTYPER |= GPIO_OTYPER_OT_8;
    GPIOB->OTYPER |= GPIO_OTYPER_OT_9;

    // Enable pull-up resistors
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR8_1);
    GPIOB->PUPDR |= GPIO_PUPDR_PUPDR8_0;
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR9_1);
    GPIOB->PUPDR |= GPIO_PUPDR_PUPDR9_0;

    // Set alternate function to AF4 (I2C1)
    GPIOB->AFR[1] &= ~(0xF << 0); 
    GPIOB->AFR[1] |= (4 << 0);    

    GPIOB->AFR[1] &= ~(0xF << 4); 
    GPIOB->AFR[1] |= (4 << 4);  
}

void bmp_i2c_config(void) {
    I2C1->CR1 &= ~I2C_CR1_PE;

    while(I2C1->CR1 & I2C_CR1_PE);


    I2C1->CR2 = (42 & 0x3F); 

    
    I2C1->CCR = 210;

    I2C1->TRISE = 43;

    I2C1->CR1 |= I2C_CR1_PE;
}

void bmp_i2c_write(uint8_t addr, uint8_t value) {
    while(I2C1->SR2 & I2C_SR2_BUSY);

    I2C1->CR1 |= I2C_CR1_START;

    while(!(I2C1->SR1 & I2C_SR1_SB));

    I2C1->DR = BMP280_I2C_ADDR << 1;

    while(!(I2C1->SR1 & I2C_SR1_ADDR));

    (void)I2C1->SR2;

    while(!(I2C1->SR1 & I2C_SR1_TXE));

    I2C1->DR = addr;

    while(!(I2C1->SR1 & I2C_SR1_TXE));

    I2C1->DR = value;

    while(!(I2C1->SR1 & I2C_SR1_BTF));

    I2C1->CR1 |= I2C_CR1_STOP;
}

uint8_t bmp_i2c_read(uint8_t reg_addr) {
    uint8_t value;

    while(I2C1->SR2 & I2C_SR2_BUSY);

    I2C1->CR1 |= I2C_CR1_START;

    while(!(I2C1->SR1 & I2C_SR1_SB));

    I2C1->DR = BMP280_I2C_ADDR << 1;

    while(!(I2C1->SR1 & I2C_SR1_ADDR));

    (void)I2C1->SR2;

    while(!(I2C1->SR1 & I2C_SR1_TXE));

    I2C1->DR = reg_addr;

    while(!(I2C1->SR1 & I2C_SR1_BTF));

    I2C1->CR1 |= I2C_CR1_START;

    while(!(I2C1->SR1 & I2C_SR1_SB));

    I2C1->DR = (BMP280_I2C_ADDR << 1) | 0x01;

    while(!(I2C1->SR1 & I2C_SR1_ADDR));

    (void)I2C1->SR2;

    I2C1->CR1 &= ~I2C_CR1_ACK;

    I2C1->CR1 |= I2C_CR1_STOP;

    while(!(I2C1->SR1 & I2C_SR1_RXNE));

    value = I2C1->DR;

    while(I2C1->CR1 & I2C_CR1_STOP);

    I2C1->CR1 |= I2C_CR1_ACK;

    return value;
}

double temperature(int x) {
    unsigned short calib_T1 = 0;
    signed short calib_T2 = 0;
    signed short calib_T3 = 0;
    signed long raw_temperature = 0;
    double var1 = 0;
    double var2 = 0;
    uint32_t t_fine = 0;
    double final_temp = 0;

    // Read calibration values
    calib_T1 = bmp_i2c_read(0x88);
    calib_T1 |= (bmp_i2c_read(0x89) << 8);

    calib_T2 = bmp_i2c_read(0x8A);
    calib_T2 |= (bmp_i2c_read(0x8B) << 8);

    calib_T3 = bmp_i2c_read(0x8C);
    calib_T3 |= (bmp_i2c_read(0x8D) << 8);

    // Read raw temperature data
    raw_temperature = bmp_i2c_read(0xFA) << 12;   // msb
    raw_temperature |= bmp_i2c_read(0xFB) << 4;   // lsb
    raw_temperature |= bmp_i2c_read(0xFC) >> 4;   // xlsb (upper 4 bits only)

    // Calculate temperature
    var1 = (((raw_temperature / 16384.0) - (calib_T1 / 1024.0)) * calib_T2);
    var2 = (((raw_temperature / 131072.0) - (calib_T1 / 8192.0))
            * ((raw_temperature / 131072.0) - (calib_T1 / 8192.0)) * calib_T3);

    t_fine = var1 + var2;
    final_temp = t_fine / 5120.0;

    printf("Final temperature: %.2f°C\n", final_temp);

    if (x == 1) {
        return t_fine;
    } else if (x == 0) {
        return final_temp;
    } else {
        return 0;
    }
}

double pressure(void) {
    double var1 = 0;
    double var2 = 0;
    unsigned short dig_P1 = 0;
    signed short dig_P2 = 0;
    signed short dig_P3 = 0;
    signed short dig_P4 = 0;
    signed short dig_P5 = 0;
    signed short dig_P6 = 0;
    signed short dig_P7 = 0;
    signed short dig_P8 = 0;
    signed short dig_P9 = 0;
    uint32_t t_fine = 0;
    signed long raw_pressure = 0;
    double p = 0;

    t_fine = temperature(1);

    dig_P1 = bmp_i2c_read(0x8E);
    dig_P1 |= (bmp_i2c_read(0x8F) << 8);

    dig_P2 = bmp_i2c_read(0x90);
    dig_P2 |= (bmp_i2c_read(0x91) << 8);

    dig_P3 = bmp_i2c_read(0x92);
    dig_P3 |= (bmp_i2c_read(0x93) << 8);

    dig_P4 = bmp_i2c_read(0x94);
    dig_P4 |= (bmp_i2c_read(0x95) << 8);

    dig_P5 = bmp_i2c_read(0x96);
    dig_P5 |= (bmp_i2c_read(0x97) << 8);

    dig_P6 = bmp_i2c_read(0x98);
    dig_P6 |= (bmp_i2c_read(0x99) << 8);

    dig_P7 = bmp_i2c_read(0x9A);
    dig_P7 |= (bmp_i2c_read(0x9B) << 8);

    dig_P8 = bmp_i2c_read(0x9C);
    dig_P8 |= (bmp_i2c_read(0x9D) << 8);

    dig_P9 = bmp_i2c_read(0x9E);
    dig_P9 |= (bmp_i2c_read(0x9F) << 8);

    // Read raw pressure
    raw_pressure = bmp_i2c_read(0xF7) << 12;   // msb
    raw_pressure |= bmp_i2c_read(0xF8) << 4;   // lsb
    raw_pressure |= bmp_i2c_read(0xF9) >> 4;   // xlsb

    // Calculate pressure
    var1 = ((double)t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
    var2 = var2 + (var1 * ((double)dig_P5) * 2.0);
    var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
    var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);

    if (var1 == 0.0) {
        return 0; // Avoid division by zero
    }

    p = 1048576.0 - (double)raw_pressure;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)dig_P9) * p * p / 2147483648.0;
    var2 = p * ((double)dig_P8) / 32768.0;
    p = p + (var1 + var2 + ((double)dig_P7)) / 16.0;

    printf("Pressure: %.2f Pa\n", p);
    return p;
}

void bmp_i2c_setup(void) {
    bmp_rcc_config();
    bmp_gpio_config();
    bmp_i2c_config();

    // Configure BMP280 to start measurements
    // 0xF4 is the control register
    // Setting: Normal mode, temperature x1, pressure x1
    bmp_i2c_write(0xF4, 0x27);

    // Filter setting (register 0xF5)
    // Setting: Filter coefficient 2
    bmp_i2c_write(0xF5, 0x0C);
}



void delay_ms(uint32_t ms) {
	SysTick->LOAD = (SystemCoreClock / 8000) * ms - 1;
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}


