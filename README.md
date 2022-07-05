# Simple event driven operating system for STM32Fxx MCU series with command line interface,
# GUI features and Tiny File System for on-chip NAND.

## Features:

* Single-threaded, even-driven API based on SVC calls.
* Command line interface allows configuration and diagnozing hardware (GPIO, PWM, ADC, UART, SPI).
* Simple shell scripting, AUTOEXEC.BAT.
* Tiny File System (TFS) for storing data and application files on on-chip NAND flash.
* ZMODEM protocol to upload files over serial (UART) port.
* Persistant configuration stored in NAND.
* Persistand environment variebles stored in battery-backup memory block.
* Built-in Modbus/RTU protocol implementation.
