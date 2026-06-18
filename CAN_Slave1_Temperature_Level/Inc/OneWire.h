/**
    @author Stanislav Lakhtin
    @date   11.07.2016
    @brief  Implementation of the 1wire protocol based on the libopencm3 library for the STM32F103 microcontroller.
            It is possible that the library will work correctly on other microcontrollers (requires verification).
            The general idea is to use the hardware USART of the microcontroller to simulate 1wire communication.
            Devices are connected to the selected USART's TX pin, which should be pulled up to the power line with a 4.7K resistor.
            The library's implementation closes RX to TX inside the microcontroller, leaving the RX pin available for other tasks.
 */

#ifndef STM32_DS18X20_ONEWIRE_H
#define STM32_DS18X20_ONEWIRE_H
#include <stdint.h>
#define ONEWIRE_NOBODY 0xF0 // Search ROM command
#define ONEWIRE_SEARCH 0xF0 // Search ROM command
#define ONEWIRE_SKIP_ROM 0xCC  // Skip ROM command
#define ONEWIRE_READ_ROM 0x33  
#define ONEWIRE_MATCH_ROM 0x55 // Match ROM command allows the master to address a specific slave device
#define ONEWIRE_CONVERT_TEMPERATURE 0x44
#define ONEWIRE_READ_SCRATCHPAD 0xBE    // Command to read the sensor's memory
#define ONEWIRE_WRITE_SCRATCHPAD 0x4E   // Command to write to the sensor's memory
#define ONEWIRE_COPY_SCRATCHPAD 0x48
#define ONEWIRE_RECALL_E2 0xB8

#ifndef MAXDEVICES_ON_THE_BUS
#define MAXDEVICES_ON_THE_BUS 1  // Maximum planned number of devices on the bus
#endif

#define DS18B20 0x28  // Sensor family code
#define DS18S20 0x10  // Sensor family code

#define WIRE_0    0x00 // 0x00 --default
#define WIRE_1    0xff // Response
#define OW_READ   0xff

typedef struct {
  int8_t inCelsius;
  uint8_t frac;
} Temperature; //

typedef struct {
  uint8_t family;
  uint8_t code[6];
  uint8_t crc;
} RomCode; //

typedef struct {
  uint8_t crc;
  uint8_t reserved[3];
  uint8_t configuration;
  uint8_t tl;
  uint8_t th;
  uint8_t temp_msb;
  uint8_t temp_lsb;
} Scratchpad_DS18B20;//

typedef struct {
  uint8_t crc;
  uint8_t count_per;
  uint8_t count_remain;
  uint8_t reserved[2];
  uint8_t tl;
  uint8_t th;
  uint8_t temp_msb;
  uint8_t temp_lsb;
} Scratchpad_DS18S20;//

typedef struct {
  RomCode ids[MAXDEVICES_ON_THE_BUS]; // For all ROMs of our sensors
  int lastDiscrepancy;
  uint8_t lastROM[8]; // Last read ROM for searching all ROMs
} OneWire;

typedef struct {
	int device;
	char info[30];
} DEVInfo;

extern DEVInfo devInfo;

void usart_setup_(uint32_t baud);

uint16_t owResetCmd(void);

int owSearchCmd(OneWire *ow);

void owSkipRomCmd(OneWire *ow);

uint8_t owCRC8(RomCode *rom);

void owMatchRomCmd(RomCode *rom);

void owConvertTemperatureCmd(OneWire *ow, RomCode *rom);

uint8_t* owReadScratchpadCmd(OneWire *ow, RomCode *rom, uint8_t *data);

void owCopyScratchpadCmd(OneWire *ow, RomCode *rom);

void owRecallE2Cmd(OneWire *ow, RomCode *rom);

Temperature readTemperature(OneWire *ow, RomCode *rom, uint8_t reSense);

void owSend(uint16_t data);

void owSendByte(uint8_t data);

uint16_t owEchoRead(void);

void owReadHandler(void);

int get_ROMid(void);

void get_Temperature(void);

#endif // STM32_DS18X20_ONEWIRE_H
