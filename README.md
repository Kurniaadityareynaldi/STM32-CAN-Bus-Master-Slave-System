# STM32 CAN Bus Multi-Node Communication

A multi-node CAN Bus communication project built with STM32F103 microcontrollers using STM32 HAL libraries.

This project demonstrates distributed communication between three STM32 nodes connected through a CAN Bus network. The system exchanges sensor data, switch states, and control commands in real-time.

## Features

### Master Node (ID: 0x446)

* Reads potentiometer value using ADC
* Sends LED brightness commands via CAN
* Sends switch status to slave nodes
* Receives temperature data from Slave 1
* Receives water level percentage from Slave 1
* Receives switch status from Slave 2
* Displays received information on SSD1306 OLED

### Slave Node 1 (ID: 0x211)

* Reads DS18B20 temperature sensor
* Reads analog water level sensor
* Controls LED brightness using PWM
* Sends temperature data to Master
* Sends water level percentage to Master
* Sends local switch status to Master

### Slave Node 2 (ID: 0x215)

* Reads local switch input
* Sends switch state to Master
* Receives switch control commands from Master
* Controls onboard LED

## System Architecture

```text
                 CAN BUS NETWORK
 ┌─────────────────────────────────────────┐
 │                                         │
 │   Master Node (0x446)                   │
 │   OLED Display                          │
 │   Potentiometer                         │
 │   Local Switch                          │
 │                                         │
 └──────────────┬──────────────────────────┘
                │
     ┌──────────┴──────────┐
     │                     │
     │                     │
┌────▼─────┐         ┌────▼─────┐
│ Slave 1  │         │ Slave 2  │
│ ID 0x211 │         │ ID 0x215 │
│           │         │           │
│ DS18B20   │         │ Switch    │
│ WaterLvl  │         │ LED Ctrl  │
│ PWM LED   │         │           │
└───────────┘         └───────────┘
```

## CAN Message IDs

| Node    | CAN ID | Function         |
| ------- | ------ | ---------------- |
| Master  | 0x446  | Control Commands |
| Slave 1 | 0x211  | Sensor Data      |
| Slave 2 | 0x215  | Switch Status    |

## Data Exchange

### Master → Slave

| Packet         | Description          |
| -------------- | -------------------- |
| LED_BRIGHTNESS | PWM brightness value |
| SWITCH_1       | Switch status        |

### Slave 1 → Master

| Packet      | Description            |
| ----------- | ---------------------- |
| TEMPERATURE | DS18B20 temperature    |
| PERCENTAGE  | Water level percentage |
| SWITCH      | Switch status          |

### Slave 2 → Master

| Data         | Description           |
| ------------ | --------------------- |
| Switch State | Digital switch status |

## Hardware Requirements

* 3 × STM32F103C8T6 (Blue Pill)
* MCP2551 or TJA1050 CAN Transceiver
* DS18B20 Temperature Sensor
* SSD1306 OLED Display (I2C)
* Potentiometer
* Water Level Sensor
* Push Buttons / Switches
* LEDs
* CAN Bus Wiring with 120Ω Termination Resistors

## Development Environment

* STM32CubeIDE
* STM32 HAL Drivers
* STM32CubeMX
* C Language

## Applications

* Industrial Monitoring Systems
* Distributed Sensor Networks
* Building Automation
* Water Tank Monitoring
* CAN Bus Learning Projects
* Embedded Systems Education

## Third-Party Components

This project uses a modified SSD1306 OLED display driver (`Drivers/SSD1306/ssd1306.c`, `ssd1306.h`, `fonts.c`, `fonts.h`) originally written by:

* **Tilen Majerle** ([tilen@majerle.eu](mailto:tilen@majerle.eu)) — original author
* **Alexander Lutsai** ([s.lyra@ya.ru](mailto:s.lyra@ya.ru)) — STM32F10x port/modification

These files are licensed under the **GNU General Public License v3 (or later)**, as stated in their original file headers. They have been lightly reformatted (indentation, comments, dead-code removal) for this project, but the license and attribution have been kept intact, as required by the GPL.

1-Wire / DS18x20 Driver

This project also uses a 1-Wire communication implementation for STM32F103 microcontrollers to interface with DS18x20 temperature sensors.

The 1-Wire implementation consists of:

Core/Inc/onewire.h
Core/Src/onewire.c

The original onewire.h source identifies the following author and date:

Stanislav Lakhtin — original author
11.07.2016 — original source date

The original source describes the implementation as a 1-Wire protocol implementation based on the libopencm3 library for the STM32F103 microcontroller. It uses the STM32 USART hardware to simulate 1-Wire communication.

The original author attribution and source header have been retained in this project.

License notice: The provided onewire.h source does not contain an explicit license statement. Therefore, this project does not assign or claim a specific license for the original 1-Wire implementation without verification of its original license.

The onewire.h and onewire.c files should retain their original copyright, author, and license notices, if present in the original source distribution.

Project Licensing

The project contains both original code and third-party components. Third-party components remain subject to their respective original license terms.

The SSD1306 driver is distributed under the GNU General Public License v3 (or later) as stated in its original source headers.

The 1-Wire / DS18x20 implementation retains its original author attribution, while its specific license has not been established from the available source header.

Because the SSD1306 driver is licensed under GPL and is compiled together with the firmware, the overall project is distributed under the GNU General Public License v3 (GPL-3.0), subject to the terms and conditions of the applicable third-party licenses.

See License below.

## Author

**Kurnia Aditya Reynaldi**

Electrical Engineer | Embedded Systems | Control Systems | Electronics R&D

Contributions, issues, and pull requests are welcome.

## License

This project is licensed under the **GNU General Public License v3.0 (GPL-3.0)**.

It was originally intended to be MIT-licensed, but because it includes and links against the third-party SSD1306 driver (`ssd1306.c`/`fonts.c`) under GPL v3, the entire combined firmware must also be distributed under GPL v3 terms. Application-specific code written for this project (`main.c`, `main.h`) is authored by Kurnia Aditya Reynaldi and is released as part of this GPL-3.0-licensed project.

See the [LICENSE](LICENSE) file for the complete license text.

```text
STM32 Water Level Monitoring System
Copyright (C) 2024  Kurnia Aditya Reynaldi

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
```
****
