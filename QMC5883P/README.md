# QMC5883L I2C Driver for STM32F4

A simple, **register-level (LL) I2C driver** for the QMC5883L magnetometer sensor, written for STM32F4 series microcontrollers (like the "Black Pill").

This driver **does not use the STM32 HAL**, relying directly on register access for peripheral configuration and I2C communication.

---

## Features

* **Low-Level (Register) Access:** No dependency on HAL or CubeMX-generated code.
* **Peripheral Configuration:** Functions to set up RCC, GPIO, and I2C1.
* **Blocking I2C:** Simple, blocking `qmc_i2c_write` and `qmc_i2c_read` functions.
* **Sensor Initialization:** Configures the QMC5883L for continuous measurement.
* **Heading Calculation:** Reads 3-axis (X, Y, Z) magnetometer data and calculates a 2D compass heading using `atan2`.
* **Debug Output:** Uses `printf` to output the sensor ID and calculated heading.

---

## Hardware Setup

* **MCU:** STM32F401, STM32F411, or other STM32F4 "Black Pill" boards.
* **Sensor:** QMC5883L Magnetometer Module (or clone).
* **Connections:**

  * PB8 (STM32F4) → SCL (QMC5883L)
  * PB9 (STM32F4) → SDA (QMC5883L)
  * 3.3V → VCC
  * GND → GND

> **Note:** The driver enables internal pull-up resistors on PB8 and PB9. For reliability, especially on long wires, **external 4.7kΩ pull-up resistors** are recommended.

---

## Software & Configuration

This driver is hard-coded with the following configuration:

* **I2C Peripheral:** I2C1
* **I2C Pins:** PB8 (SCL), PB9 (SDA)
* **I2C Mode:** Standard Mode (100kHz)
* **APB1 Clock:** Assumes PCLK1 = 42MHz. If different, update `I2C1->CR2` and `I2C1->CCR` in `qmc_i2c_config()`.

---

## How to Use

1. **Add Files:** Add `qmc5883l.c` to your project's source files.
2. **Create Header:** Create `qmc5883P.h` for the sensor address and function prototypes.

### Sample `qmc5883P.h`

```c
#ifndef QMC5883P_H_
#define QMC5883P_H_

#include "stm32f4xx.h"
#include <stdint.h>

/* QMC5883L Default I2C Address (7-bit) */
#define QMC_ADDR 0x0D

// --- Public Function Prototypes ---

void qmc_rcc_config(void);
void qmc_gpio_config(void);
void qmc_i2c_config(void);
void qmc_init(void);
void qmc_mag_read(void);

// --- Low-Level I2C Functions ---
void qmc_i2c_write(uint8_t addr, uint8_t value);
uint8_t qmc_i2c_read(uint8_t reg_addr);

#endif /* QMC5883P_H_ */
```

### Sample `main.c`

```c
#include "stm32f4xx.h"
#include "qmc5883P.h"

// Simple blocking delay
void crude_delay(volatile uint32_t count) {
    while (count--);
}

int main(void) {
    // 1. Configure System Clocks
    //    (HSE, PLL, SYSCLK 84/100MHz, PCLK1 42MHz)

    // 2. Configure UART for printf
    //    (syscalls.c retarget printf to USART2)

    printf("System Init Complete.\r\n");

    // 3. Initialize QMC5883L driver
    qmc_rcc_config();
    qmc_gpio_config();
    qmc_i2c_config();

    // 4. Initialize sensor
    qmc_init();

    while (1) {
        // 5. Read magnetometer data and print heading
        qmc_mag_read();

        // Delay (~100-200ms for 200Hz sensor)
        crude_delay(1000000);
    }
}
```

---

## Dependencies

* `stm32f4xx.h` – CMSIS/LL header for STM32F4 series
* `stdio.h` – For `printf` (requires UART retargeting)
* `math.h` – For `atan2` in heading calculation (`-lm` linker flag may be required)

---

## Notes

* Sensor operates at **200Hz ODR**, so reading every 100–200ms is reasonable.
* The code is **blocking**; for high-performance or low-power applications, consider using **interrupt-driven I2C** or **DMA
