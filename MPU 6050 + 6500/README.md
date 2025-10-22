

![3886-07](https://github.com/user-attachments/assets/c4c7ee42-3861-4f02-9e3d-b9bcecd4d064)
# MPU6050 Driver (CMSIS / STM32F4 Blackpill)

This project contains a **bare-metal CMSIS-based driver** for the **MPU6050 (3-axis accelerometer + 3-axis gyroscope)**, written for STM32F4 **Blackpill boards** (STM32F401/STM32F411).

The driver is implemented without using STM32 HAL/LL — it directly configures registers for **RCC**, **GPIO**, and **I²C1** peripherals.

---
![Block-Diagram-of-MPU6050](https://github.com/user-attachments/assets/d3361f5f-fd8b-440e-96a6-64bc9344f80b)

## Features

* Pure **CMSIS / register-level** implementation (no HAL/LL)
* Initializes MPU6050 over **I²C1 (PB8 = SCL, PB9 = SDA)**
* Provides:

  * Raw accelerometer & gyroscope data (`Accel_X_RAW`, `Gyro_X_RAW`, …)
  * Scaled values (`Ax, Ay, Az` in *g*, `Gx, Gy, Gz` in *°/s*)
* Functions for reading **all axes** or **single-axis values**
* Simple `printf` debug output for serial plotters (e.g. Arduino IDE)

---
## Usage

1. Connect MPU6050 to STM32 Blackpill:

   * **PB8 → SCL**
   * **PB9 → SDA**
   * **3.3V / GND**
2. Include header in your `main.c`:

   ```c
   #include "mpu6050.h"
   ```
3. Initialize peripherals + sensor:

   ```c
   int main(void) {
       mpu_start();   // RCC + GPIO + I2C init + MPU initial Init Settings
      
       while (1) {
           mpu_accel_read();
           mpu_gyro_read();
           //or any of the individual x,y,z read functions for the accel/gyro
       }
   }
   ```
4. View live data over UART using `printf` (can be piped to Arduino IDE Serial Plotter).

---

## Functions

* **Initialization**

  * `mpu_start()` – Enable clocks, GPIO, and I²C1
  * `mpu_init()` – Verify WHO\_AM\_I, configure registers
* **Accelerometer**

  * `mpu_accel_read()` – Read all axes, update `Ax, Ay, Az`
  * `mpu_accel_read_x/y/z()` – Read individual axis
* **Gyroscope**

  * `mpu_gyro_read()` – Read all axes, update `Gx, Gy, Gz`
  * `mpu_gyro_read_x/y/z()` – Read individual axis
* **I²C Helpers**

  * `imu_i2c_write(addr, val)` – Write to register
  * `imu_i2c_read(addr)` – Read from register

---

## Dependencies

* **CMSIS** (e.g. included with STM32CubeIDE)
* UART configured for `printf` (for serial debugging/plotting)

---

##  Notes

* MPU6050 I²C address used: `0x68` (default, AD0 = GND)
* Accelerometer scale: **±2g** → `16384 LSB/g`
* Gyroscope scale: **±250°/s** → `131 LSB/(°/s)`
* Modify in code if different full-scale ranges are desired.

---

## Author

**lawslefthand** – *August 16, 2025*

---
