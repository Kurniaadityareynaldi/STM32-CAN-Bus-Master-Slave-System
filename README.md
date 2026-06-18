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

## License

This project is licensed under the MIT License. See the LICENSE file for details.
