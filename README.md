# DS3231 RTC Project

This project demonstrates how to use the DS3231 Real-Time Clock (RTC) module on a Raspberry Pi 4 using the pigpio library. 

## Authors and contributors

This is an assignment implementation by **Arina Sofiyeva**, based on the original work and guidance of [Derek Molloy](https://github.com/derekmolloy).

Main author: Derek Molloy

Contributors: Arina Sofiyeva

## Features

- Sets the current time and date on the RTC module (in both 12h and 24h modes)
- Reads and displays the current RTC module time, date and temperature. 
- Sets and reads two alarms. One is triggered when hours, minutes, seconds and specific days match. The second one is triggered when minutes, hours and specific dates match. (All trigger in 1 minute ahead of the current time)
- Demonstrates I2C communication with the RTC on Raspberry Pi. 
- Uses RTC interrupts to control LED blinking, based on alarm triggers.
- Drives PWM-controlled LED brightness depending on SQW frequency (1Hz - LED at 100% PWM, 1.024kHz - LED at 75% PWM, 4.096kHz - LED at 50% PWM, 8.192kHz - 25% PWM).
- Uses `pigpio` for GPIO control.

### The full explanation is [here.](https://docs.google.com/document/d/1f_G5BnIo9p2eZKWcX1IQhufwqVfJLneRRVD_5da55mM/edit?usp=sharing) 

The grade received: 94/100.

## Requirements

- Raspberry Pi 4  
- DS3231 RTC module 
- `pigpio` library installed and running
- One LED 
- One resistor

## Installation

Clone the repository:

```bash
git clone https://github.com/heroisaprinciple/ds3231RTCProject.git
cd ds3231RTCProject
```

Build the project:

```bash
./build
```

> The `build` script compiles application.cpp, I2CDevice.cpp and DS3231.cpp into a single executable rtc using g++ with flags.

## Usage

Start the `pigpiod` daemon if it is not already running:

```bash
sudo pigpiod
```

Then run the program with superuser privileges:

```bash
sudo ./rtc
```

> Superuser access is required because `pigpio` performs low-level GPIO operations.

Choose a time format in the console menu and enjoy!

## Demo

### The videos:
- Part 1. 24h mode: https://drive.google.com/file/d/1Z2AjoxirsHOiT-u8FgImH-B_GP-E4pXV/view?usp=sharing
- Part 2. 12h mode: https://drive.google.com/file/d/1ykMywWrtRKMtZHHz3JlNjtWfhz4C3LkY/view?usp=sharing
- Result: https://drive.google.com/file/d/1ipUiKreYtVdCXzoaIvkV0XJMF4RwG29j/view?usp=sharing

## References

- Derek Molloy, [*Exploring Raspberry Pi*](https://github.com/derekmolloy/exploringrpi)

## License
Copyright 2025- Dublin City University
