# GroveOS is an event-driven single-threaded operating system for ARM Cortex-M4F based microcontrollers

The purposes of this tiny OS are as follows:

1. To abstract hardware layer from firmware developer to ease and speed-up development of bare-metal applications.
2. To resolve licensing issues: all third-party (GPL) code is part of the OS itself, which allows applications to be license-free.
3. To ease application updates: developers can distribute ELF binaries of their modules (applications), collateral resource and config files which can be easily uploaded to built-in Flash of the hardware using serial ZModem protocol. Downloading back from device is not possible.

## Supported hardware

* Currently only STmicro's STM32F4xxx series are supported.
* We are working on adding support for other microcontrollers, mainly GigaDevice GD32F4xxx series.

## Features:

* Single-threaded, event-driven API based on SVC calls. Applications (modules) are standard ELF32 files, must be built for GroveOS using SVC API.
* Command line interface allows configuration and probing hardware (GPIO, PWM, ADC, UART, SPI).
* Simple shell scripting, AUTOEXEC.BAT.
* Tiny File System (TFS) for storing data and application files on on-chip NAND flash.
* ZMODEM protocol to upload files over serial (UART) port.
* System configuration is stored in NAND.
* Environment variebles are stored in battery-backup memory block.
* Built-in Modbus/RTU protocol implementation, allows both master and slave operation.
* DALI protocol implementation.
* SPI LCD support with on-screan resistive touch (FT800).
* Winstar OLED display support.
* GUI widgets: windows, buttons, input fields, on-screan keyboard.
* Software timers with 1ms granularity.


## Dependancies:

1. GNU C Toolchain 4.7 for ARM32 cross-compiler (gcc-arm-none-eabi-4_7-2013q3).
2. STmicros's STM32 libraries: CMSIS, STM32F4xx_StdPeriph_Driver, STM32_USB_HOST_Library.
 

## Building GroveOS

1. Install GCC cross-compiler for ARM32 bare-metal from https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain

2. Install STmicros's STM32 libraries from https://github.com/Fabmicro-LLC/STM32Libraries.git

3. Modify Makefile to match paths to installed GCC cross-compiler and to STM32 libraries.

4. Make a copy of hardware.h and modify it to meet your hardware requirements.

5. Add your own target to Makefile which includes your modified version of hardware.h, see 'aps' or 'grovex' targets for reference.

6. Run ```make <your_target>``` command.

7. Upload resulting ```main.hex``` to your microcontroller using ST-Link utility or use ```stm32flash```:

```stm32flash -w main.hex -v -b 115200 /dev/ttyUSB0```

8. Connect your terminal (```minicom```, ```cu``` or ```hyperterm.exe```) to device over serial port using 115200 8N1, reset/powerup the device and look through the boot process. After success boot, which usually takes 5-7 seconds, you will get to GroveOS command prompt.

9. Use ```HELP``` command to list of available commands and their syntax.


## Application examples

An example of GroveOS module (application) can be found in ```GroveOS_Module_Monitor/``` subdirectory of this code tree, please read corresponding README.

