![4877_top_ORIG_2021_03](https://github.com/user-attachments/assets/c56c5b9e-4118-4d77-b84d-b40d3f0ff82d)
## STM32 Blackpill CMSIS Drivers

A comprehensive collection of **CMSIS** and selective **HAL-based drivers** written in C for the STM32F401CEU-powered **Blackpill** development boards. This repository focuses on providing **clean, modular, and reusable drivers** for commonly used peripherals, enabling developers to have full control over the hardware.

While primarily targeted at the STM32F401CEU, these drivers are also compatible with the STM32F411CEU with minimal modifications (mainly in clock configuration).

---

## Key Features

* **Modular and Scalable Architecture** – Each driver is implemented independently, making it easy to integrate only what is needed.
* **Optimized for Performance** – Minimal abstraction layers to ensure low overhead and high efficiency.
* **Portable Across STM32F4 Series** – Code can be easily adapted to other MCUs in the STM32F4 family.
* **Blackpill-Oriented Testing** – Verified and tested on STM32F401CEU-based Blackpill boards.
* **CMSIS-Level Control** – Direct register-level access with the option to mix with HAL if needed.

---
## Board Pinout
F411CCU:
<img width="1280" height="739" alt="STM32-STM32F4-STM32F411-STM32F411CEU6-pinout-low-resolution png" src="https://github.com/user-attachments/assets/4248c9c3-d492-4764-b207-18cf1837b7a2" />
F401CEU:
<img width="1280" height="779" alt="STM32-STM32F4-STM32F401-STM32F401CCU6-pinout-low-resolution jpg" src="https://github.com/user-attachments/assets/6a1b8725-3bf2-4acb-b415-4d1c36f9c89c" />


---

## Documentation

Each driver is documented within its respective header file (`.h`). The documentation includes:

* Register definitions and explanations
* Usage examples
* Configuration options
* Integration notes for combining multiple drivers

Additionally, example projects are provided in the `examples/` directory to help new users get started quickly.

---

## Getting Started

Simply copy the .c and .h files for the drivers you want to use in you src and inc folders in the Stm32CUBEIDE project directory.

---

## Contributions

We welcome contributions of any kind:

* New driver implementations
* Example projects
* Bug fixes and optimizations
* Documentation improvements

Before submitting a pull request:

* Follow the existing code style
* Provide adequate documentation in the header files
* Test the changes on hardware where possible

---

## License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for full details.

---


