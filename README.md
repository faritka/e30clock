# BMW E30 clock software for Zephyr RTOS and STM32

This repository contains software for running the BMW E30 6-button clock with new hardware.

The display is based on 5 dot-matrix 5x7 red LED displays LTP-305HR. They are controlled by the HT1632C chip.

The main microcontroller for the clock is the STM32L452RET6. It controls the display chip using 4 lines CS (chip select), 
WR (write clock), RD (read clock) and DATA (serial data). 6 push buttons are connected to the microcontroller GPIO inputs 
and call interrupt functions when they are pushed on. The microcontroller saves configuration data into a ROM chip via the I2C protocol.

The light sensor VEML-7700 detects a light level and returns the value to the microcontroller via I2C. Then, the microcontroller adjusts 
the brightness of the display. At night, the brightness level is the lowest. At a sunny day, the brightness level is the highest.

The real-time clock module is the RV-3032-C7, which is temperature-compensated and very accurate. When the main power source of 
an automobile is turned off, it switches to the super-capacitor supply. The charging of the super-capacitor is controlled by the LTC4079.

The ADC in the STM32 can read the voltage level from the connected BMW thermistor and calculate the current temperature.

The clock can play sounds on the BMW gong. The optocoupler is used to trasfer the different level voltages from 
the microcontroller (3.3V) to the car systems (12V).

The software uses the Real-Time Operating System Zephyr to organize multiple threads, transfer signals between threads, and simplify
writing drivers.

## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment. You can follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

### Documentation

[BMW E30 clock for Zephyr RTOS and STM32](http://hobby.farit.ru/bmw-e30-clock-zephyr-stm32/).

### Initialization

The first step is to initialize the workspace folder (``my-workspace``) where
the ``e30clock`` and all Zephyr modules will be cloned. You can do
that by running:

```shell
# initialize my-workspace for the e30clock application (main branch)
west init -m https://github.com/faritka/e30clock --mr main my-workspace
# update Zephyr modules
cd my-workspace
west update
```

### Configuration

The application is based on a custom board that fits inside of a clock.

### Build & Run

The application can be built by running:

```shell
west build -b $BOARD -s app
```

where `$BOARD` is the target board.

Once you have built the application you can flash it by running:

```shell
west flash
```
