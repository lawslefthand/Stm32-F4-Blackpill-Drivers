

#ifndef OLED_H_
#define OLED_H_

#include <stdio.h>
#include "stm32f4xx.h"
#include <stdint.h>


#define GPIOBEN (1U<<1)
#define I2C1EN (1U<<21)

#define SR2_BUSY (1U<<1)
#define SR1_SB (1U<<0)
#define SR1_ADDR (1U<<1)
#define SR1_TXE (1U<<7)
#define SR1_BTF (1U<<2)

#define CR1_START (1U<<8)
#define CR1_PE (1U<<0)
#define CR1_STOP (1U<<9)

#define SH1106_ADDR 0x3C  // i2c address of SH1106.If you don't know address run i2c_scanner()


void delay_ms(uint32_t ms);
void I2C1_init(void);
void i2cWriteByte(uint8_t addr, uint8_t control, uint8_t value);
void i2cBurstWrite(char saddr, int n, char* data);
void i2c_scanner(void);
void cmdwrite(uint8_t cmd);
void datawrite(uint8_t data);
void SH1106_init(void);
void clear(void);
void fill(void);
void setcursor(uint8_t x, uint8_t page);
void writechar(char ch, uint8_t page, uint8_t x);
void drawBitmap(uint8_t x, uint8_t page, uint8_t width, uint8_t height, const uint8_t *bitmap);
void write(uint8_t x, uint8_t page, const char* str);
void drawline_H(uint8_t x, uint8_t y, uint8_t width);
void drawline_H(uint8_t x, uint8_t y, uint8_t height);
void rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void invert_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void draw_arrow(uint8_t x,uint8_t y);

#endif
