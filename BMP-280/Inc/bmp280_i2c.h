/*
 * bmp280_i2c.h
 *
 *  Created on: Dec 15, 2024
 *      Author: danba
 */

#ifndef SRC_BMP280_I2C_H_
#define SRC_BMP280_I2C_H_

void bmp_rcc_config(void);
void bmp_gpio_config(void);
void bmp_i2c_config(void);
void bmp_i2c_write(uint8_t addr, uint8_t value);
uint8_t bmp_i2c_read(uint8_t tx_addr);
double temperature(int x);
double pressure(void);

void delay_1s(void);
void trim_read(void);
void bmp_i2c_setup(void);
void delay_ms(uint32_t ms);


#endif /* SRC_BMP280_I2C_H_ */

