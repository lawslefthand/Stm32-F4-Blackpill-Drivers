![H8af206d54d1d470a9d2e80f7071b29edf (1)](https://github.com/user-attachments/assets/0ab94cac-6292-4cb5-8d99-64ad7a79bf00)


# SH1106 Driver for STM32F4 Boards

The **SSD1106 OLED Display** is a compact and high-contrast screen ideal for DIY electronics, IoT devices, and embedded projects. 

This a **I2C driver** for **STM32F4** to communicate and display data on screen. The driver was completely written in **bare metal** using **CMSIS core**.

Currently, driver comes with a single font, but custom or new fonts can be added in `font.h`

---

## Features

* **Low-Level (Register) Access:** No dependency on HAL or CubeMX-generated code.
* **Peripheral Configuration:** Functions to set up RCC, GPIO, and I2C1.
* **SH1106 Initialization:** Configures the SH1106 for displaying data.
* **I2C Scanner** Tries different addresses from 0 to 126 sequentially and if any address matches, outputs it using printf(Serial Wire viewer)
* **Debug Output:** Uses `printf` to debug initialization and i2c address if i2c scanner is executed.
* **Graphics:** This driver is able to draw rectangles, inverted rectangles, horizontal & vertical lines at a particular `x` and `y` values.
* **Bitmaps:** Can draw bitmaps using raw byte data.

---

## Hardware Setup
<img width="533" height="483" alt="image"  src="https://github.com/user-attachments/assets/282f0692-f86e-46d4-9e17-1df5f5c2b725" style="display: block; margin: 0 auto" />

* **MCU:** STM32F401, STM32F411, or other STM32F4 "Black Pill" boards.
* **Display:** SH1106 OLED 
* **Connections:**

  * PB8 (STM32F4) → SCL (SH1106)
  * PB9 (STM32F4) → SDA (SH1106)
  * 3.3V → VCC
  * GND → GND

> **Note:** The driver enables internal pull-up resistors on PB8 and PB9. For reliability, especially on long wires, **external 4.7kΩ pull-up resistors** are recommended.

---

## Software & Configuration

This driver is hard-coded with the following configuration:

* **I2C Peripheral:** I2C1
* **I2C Pins:** PB8 (SCL), PB9 (SDA)
* **I2C Mode:** Fast Mode (400kHz)
* **APB1 Clock:** Assumes PCLK1 = 16MHz. If different, update `I2C1->CR2` and `I2C1->CCR` in `I2C1_init()`.

---

## How to Use

1. **Source Files:** Add `SH1106.c` to your project's source.
2. **Header Files:** Add `OLED.h` and `font.h` to your project's Include folder.

>**Note:** if you include `OLED.h` or `font.h` and they are not present in same directory or even after including them in Inc folder, compiler is thowing error `No such file or directory`. Put the location where both header files are present to include paths of your project. problem will be solved. 

---

## Dependencies

* `stm32f4xx.h` – CMSIS/LL header for STM32F4 series
* `stdio.h` – For `printf` (requires UART retargeting)
* `stdint.h` - Provides fixed-width integer types

---

## Notes

* Font provided in `font.h` is 8x8 pixels. If you want to use your own font, create a C style array of it(make sure byte oreintation is vertical).
* SH1106 only supports vertical addressing (page-oriented) only.Modify `writechar()` if needed.
* I didn't experiment much on custom font or other fonts, but you can sure try. If any bugs spotted or any addition from your side, feel free to contact me.


