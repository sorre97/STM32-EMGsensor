![GPL license](https://img.shields.io/badge/license-GPL-blue.svg)
![Generic badge](https://img.shields.io/badge/build-pass-green.svg)
![GitHub last commit](https://img.shields.io/github/last-commit/sorre97/STM32-EMGsensor)

![Generic badge](https://img.shields.io/badge/RTOS-miosix-orange.svg)
![made-with-c++](https://img.shields.io/badge/Made%20with-C/C++-1f425f.svg)

# Embedded EMG singal processing

## Overview
This application uses on-board ADC to convert analog input from an EMG sensor and performs digital signal processing filtering to eliminate frequencies out of the (30hz, 300hz) interval and 50hz disturbance. Processed samples are then sent via serial communication to the PC in order to be plotted. The application has been developed on top of the __Miosix__ embedded operative system which provided Real-time operating system (RTOS) features.

## Requirements
- STM32F4x nucleo board (project has been developed using _STM32F401re nucleo_ board)
- For embedded software compilation:
  - [Miosix toolchain](https://miosix.org/wiki/index.php?title=Miosix_Toolchain) (for cross compilation)
  - [GNU ARM embedded toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) (linker script)

## External libraries
- [Eigen C++ template library](https://eigen.tuxfamily.org/index.php?title=Main_Page) (linear algebra operations, already embedded in the project)

## How to use
[ 1 ] Connect microcontroller to the PC with the USB cable and open serial communication to read incoming values.

[ 2 ] Connect sensor to analog pin __PA0__ and ground to __GND__.

## Utility
[ - ] Possibilities for serial reading:
- [`PuTTY`](https://putty.org) (Windows, free) 
- [`Serial`](https://www.decisivetactics.com/products/serial/) (Mac, $39.99)
- `ls -l /dev/ttyX` where X is the tty relative to serial port

[ - ] Use command `make all` in the `miosix-kernel` folder for embedded software compilation.

[ - ] Use `st-flash write main.bin 0x08000000` command to flash generated bin executable into the board.


