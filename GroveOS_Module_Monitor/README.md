# Monitor module for GroveOS

* Monitor module serves as an example of quite complex application for GroveOS.

* Monitor allows allow to control status of a set of analog sensors on Grovex hardware platfom. Monitor module can me embedded into GroveOS itself and use as default application for the OS - built and copy resulting monitor.elf to GroveOS source tree and re-build it.

## Features

* Supports Winstar OLED display to show menu.
* Complex navigation menu to view current sensor values, set/reset GPIOs.
* Implements simple Automatic Power Backup Switch: monitors input 3-phase mains power and its quality, if fauly voltage detected a switch to backup line is performed. 
* Supports remote control via Modbus/RTU prorocol.


## Dependancies:

1. GNU C Toolchain 4.7 for ARM32 cross-compiler (gcc-arm-none-eabi-4_7-2013q3).

2. STmicros's STM32 libraries: CMSIS, STM32F4xx_StdPeriph_Driver, STM32_USB_HOST_Library.


## Building module for GroveOS

NOTE: You do not need full set of GroveOS source codes to build this module except a banch of header files which are already part of this source.

1. Install GCC cross-compiler for ARM32 bare-metal.

2. Install STmicros's STM32 libraries.

3. Modify Makefile to match paths for GCC and STM32 libraries.

4. Run ```make clean && make``` command.

5. Resluting ```monitor.elf``` file can be uploaded to device using ZModem protocol.


## Using module

1. Upload ```monitor.elf``` to device using ZModem protocol.

2. Run module on device by issuing ```RUN monotor.elf``` command in GroveOS command prompt.

3. To set as default application on the device, issue ```DEFAULT monotor.elf``` commannd, then ```SAVE``` command to save settings to Flash.




