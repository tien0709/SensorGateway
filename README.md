# Gateway Firmware for STM32F411

This project is a reference embedded software system designed for STM32F411 microcontroller using C++ and FreeRTOS. It includes a bootloader and a main application with features suitable for an embedded gateway. The architecture supports future extension with debug modes, factory reset, and protocol expansion (CAN, UART, SPI, I2C).
---

## 🧠 Features

### Bootloader
- Written in C for minimal size
- Supports jumping to main / debug / factory apps
- Flag-based boot selection stored in flash
<!-- - CRC check stub for firmware validation -->

### Application (FreeRTOS + C++)
- Task-based architecture using C++ OOP classes
- UART CLI task: send command over UART
- Sensor manager task: communicates with peripheral sensor via SPI
- Logger module: stores logs in internal RAM

### Inter-task Communication
- `Queue` for transferring sensor data from sensor task to CLI
- `Mutex` to protect shared log buffer
<!-- - Designed for expansion with binary semaphores / event groups -->

### Logger
- Lightweight RAM-based logging system
- Mutex-protected buffer for multi-task logging
<!-- - Extendable for Flash or SD log persistence -->

<!-- ### Python Test Tool
- Simple CLI using `pyserial`
- Send commands: `status`, `read sensor`, `reset` -->

---

## 🔧 Build & Flash

### Using STM32CubeIDE
1. Open the project folder in CubeIDE
2. Select the target project (bootloader or application_main)
3. Build and flash to your STM32F411 board

### Bootloader Flashing
Ensure the bootloader is flashed to `0x08000000`, then flash the main app at `0x08004000`.

---

## 💬 UART Commands (via UARTCLI)

| Command         | Description              |
|----------------|--------------------------|
| `status`       | Show system status       |
| `read sensor`  | Fetch sensor value       |
| `reset`        | Reboot the MCU           |

---

## 📚 Dependencies

- FreeRTOS
- STM32 HAL (STM32CubeMX generated)
<!-- - pyserial (for Python testing) -->

---


## 📚 References

- [STM32F411 Reference Manual (RM0383)](https://www.st.com/resource/en/reference_manual/dm00119316.pdf)
- [STM32F411 Datasheet](https://www.st.com/resource/en/datasheet/stm32f411ce.pdf)
- [ARM Cortex-M4 Generic User Guide](https://developer.arm.com/documentation/dui0553/latest/)
- [Discovery kit with STM32F411VE MCU User Manual](https://www.st.com/resource/en/user_manual/um1842-discovery-kit-with-stm32f411ve-mcu-stmicroelectronics.pdf)

---

## 🚀 Future Enhancements

- Implement CRC32 check in bootloader
- Add SD card support and FatFS
- Add CAN communication task
- Add firmware update over UART
- Expose logger buffer via UART CLI



