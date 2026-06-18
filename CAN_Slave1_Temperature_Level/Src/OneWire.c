/**
    @author Stanislav Lakhtin
    @date   11.07.2016
    @brief  Implementation of the 1-Wire protocol based on the libopencm3 library for the STM32F103 microcontroller.
            The library may work correctly on other microcontrollers (verification is required).
            Verification is needed to ensure the correct configuration of UART/USART for half-duplex operation.
            The general idea is to use the microcontroller's hardware USART to emulate 1-Wire operation.
            Devices are connected to the selected USART's TX pin, which should be pulled up to the power line with a 4.7K resistor.
            The library's implementation closes RX to TX inside the microcontroller, leaving the RX pin available for other tasks.
            The library's implementation assumes possible simultaneous operation with independent buses for all possible UART/USARTs on the microcontroller.
            At the same time, all buses (up to 5 of them) will be addressed and polled individually.
 */


#include "OneWire.h"
#include "stm32f1xx_hal.h"
#include "stdio.h"
#include "string.h"
volatile uint8_t recvFlag;
volatile uint16_t rc_buffer[5];

extern UART_HandleTypeDef huart2;
#define ow_uart huart2
#define OW_USART USART2
#define MAXDEVICES_ON_THE_BUS 1

/*********************************************************************************************/
float Temp[MAXDEVICES_ON_THE_BUS];
float temperature;

uint8_t devices;
OneWire ow;
uint32_t pDelay = 300, i;
uint8_t sensor;

Temperature t;

DEVInfo devInfo;


char *crcOK;

uint16_t USART_ReceiveData(USART_TypeDef* USARTx)
{
  /* Check the parameters */
  assert_param(IS_USART_ALL_PERIPH(USARTx));

  /* Receive Data */
  return (uint16_t)(USARTx->DR & (uint16_t)0x01FF);
}

void USART_SendData(USART_TypeDef* USARTx, uint16_t Data)
{
  /* Check the parameters */
  assert_param(IS_USART_ALL_PERIPH(USARTx));
  assert_param(IS_USART_DATA(Data));

  /* Transmit Data */
  USARTx->DR = (Data & (uint16_t)0x01FF);
}


uint8_t getUsartIndex(void);

void usart_setup(uint32_t baud) {

	ow_uart.Instance = OW_USART;
	ow_uart.Init.BaudRate = baud;
	ow_uart.Init.WordLength = UART_WORDLENGTH_8B;
	ow_uart.Init.StopBits = UART_STOPBITS_1;
	ow_uart.Init.Parity = UART_PARITY_NONE;
	ow_uart.Init.Mode = UART_MODE_TX_RX;
	ow_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	ow_uart.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_HalfDuplex_Init(&ow_uart) != HAL_OK)
	{
		//	    Error_Handler();
		__asm__("NOP");
	}

	__HAL_UART_ENABLE_IT(&ow_uart, UART_IT_RXNE);
}

void owInit(OneWire *ow) {
  int i=0, k = 0;
  for (; i < MAXDEVICES_ON_THE_BUS; i++) {
   uint8_t *r = (uint8_t *)&ow->ids[i];      
    k=0;
    for (; k < 8; k++)
    r[k] = 0;
  }
  k=0;
  for (; k < 8; k++)
    ow->lastROM[k] = 0x00;
  ow->lastDiscrepancy = 64;

}

void owReadHandler() { // USART interrupt handler
  uint8_t index = getUsartIndex();
  /* Check if the interrupt was triggered by RXNE. */
  if (((OW_USART->CR1 & USART_CR1_RXNEIE) != 0) &&
      ((OW_USART->SR & UART_FLAG_RXNE) != (uint16_t)RESET)) {

    /* Retrieve data from the peripheral and clear the flag */
		while ((OW_USART->SR & UART_FLAG_RXNE) == (uint16_t)RESET){;}
    rc_buffer[index] = USART_ReceiveData(OW_USART);              
    recvFlag &= ~(1 << index);// Clear the flag indicating that a response has been received
  }
}

/** Implementation of RESET on the 1-Wire bus
 *
 * @param N usart -- USART selected for 1-Wire emulation
 * @return 1 if a device is present on the bus, 0 otherwise
 */

  uint16_t owResetCmd() {
	uint16_t owPresence;
	
	usart_setup(9600);

  owSend(0xF0); // Send RESET pulse
  owPresence = owEchoRead(); // Wait for PRESENCE on the bus and return whether a device is present

	usart_setup(115200);// Reset UART speed
  return owPresence;
}

uint8_t getUsartIndex() {// Determine which UART to use
//	uint8_t result;
//	if(OW_USART==USART1)result = 0;
//	else if (OW_USART==USART2)result = 1;
//	else if (OW_USART==USART3)result = 2;
	return 0;
}

void owSend(uint16_t data) {
  recvFlag |= (1 << getUsartIndex());// Set the flag if we enter the interrupt handler; it will be cleared there
  USART_SendData(OW_USART, data);// Send data
	while(__HAL_UART_GET_FLAG(&ow_uart, UART_FLAG_TC) == RESET);// Wait until transmission is complete
}

uint8_t owReadSlot(uint16_t data) {// Read whether a one or zero was received in response
  return (data == OW_READ) ? 1 : 0; // If 0xFF is received, the bit is 1, otherwise it's 0
}

uint16_t owEchoRead() {
  uint8_t i = getUsartIndex();// Get the USART number
  uint16_t pause = 1000;
  while (recvFlag & (1 << i) && pause--);// Wait until someone responds, but not more than the pause time
  return rc_buffer[i];// Depending on the used USART number
}

uint8_t *byteToBits(uint8_t ow_byte, uint8_t *bits) {// Decompose 1 byte into 8 bytes, encode them for 1-Wire transmission
  uint8_t i;
  for (i = 0; i < 8; i++) {
    if (ow_byte & 0x01) {// If the current bit in the byte is 1
      *bits = WIRE_1; // Replace it with a value that represents 1 in USART transmission for 1-Wire (0xFF)
    } else {
      *bits = WIRE_0;// Similarly, for 0
    }
    bits++;
    ow_byte = ow_byte >> 1; // Shift the processed bit
  }
  return bits; // Return the array for transmission
}

/**
 * Sends 8 bytes sequentially, one for each bit in data
 * @param usart -- USART selected for 1-Wire emulation
 * @param d -- data
 */
void owSendByte(uint8_t d) {
  uint8_t data[8];
	int i;
  byteToBits(d, data);// Convert the byte into bits (an array of bytes for UART transmission and 1-Wire emulation)
  for (i = 0; i < 8; ++i) {
    owSend(data[i]);
  }
}

uint8_t bitsToByte(uint8_t *bits) {// Accepts a "encoded" byte array received via UART and converts it back into a byte
  uint8_t target_byte, i;
  target_byte = 0;
  for (i = 0; i < 8; i++) {
    target_byte = target_byte >> 1;
    if (*bits == WIRE_1) {// If 0xFF was received via USART, it represents a 1
      target_byte |= 0x80;// Set the most significant bit to 1
    }
    bits++;// Move to the next byte, which is either 0x00 or 0xFF
  }
  return target_byte; // Return the received byte
}

/* Calculate the CRC8 of an array mas with length Len */
uint8_t owCRC(uint8_t *mas, uint8_t Len) {
  uint8_t i, dat, crc, fb, st_byt;
  st_byt = 0;
  crc = 0;
  do {
    dat = mas[st_byt];
    for (i = 0; i < 8; i++) {  // Bit counter in the byte
      fb = crc ^ dat;
      fb &= 1;
      crc >>= 1;
      dat >>= 1;
      if (fb == 1) crc ^= 0x8c; // Polynomial
    }
    st_byt++;
  } while (st_byt < Len); // Byte counter in the array
  return crc;
}

uint8_t owCRC8(RomCode *rom){
  return owCRC((uint8_t*)rom, 7);                        
}

/*
 * return 1 if has got one more address
 * return 0 if hasn't
 * return -1 if error reading happened
 *
 * Rewrite using callback functions to handle errors.
 */
int hasNextRom(OneWire *ow, uint8_t *ROM) {//
    uint8_t ui32BitNumber = 0;
    int zeroFork = -1;
    uint8_t i = 0;
    if (owResetCmd() == ONEWIRE_NOBODY) { // Check if there is any device on the bus
        return 0;
    }
    owSendByte(ONEWIRE_SEARCH);//
    do {
        uint8_t answerBit = 0;
        int byteNum = ui32BitNumber / 8;
        uint8_t *current = (ROM) + byteNum;
        uint8_t cB, cmp_cB, searchDirection = 0;
        owSend(OW_READ); // Read the direct bit
        cB = owReadSlot(owEchoRead()); // Response from the sensor
        owSend(OW_READ); // Read the inverted bit
        cmp_cB = owReadSlot(owEchoRead()); // Response from the sensor
        if (cB == cmp_cB && cB == 1) // Compare the two responses
            return -1; // Error, no response from any device
        if (cB != cmp_cB) { // Normal situation, received either 10 or 01
            searchDirection = cB; // Choose the direction to move forward
        } else { // Collision, received 00, meaning the current bit in ROMs is different
            if (ui32BitNumber == ow->lastDiscrepancy) // If the current collision position is equal to the previous one
                searchDirection = 1; // Choose the direction to move forward
            else {
                if (ui32BitNumber > ow->lastDiscrepancy) { // If we have moved forward
                    searchDirection = 0; // Choose the direction to move forward
                } else {
                    searchDirection = (uint8_t) ((ow->lastROM[byteNum] >> ui32BitNumber % 8) & 0x01);
                }
                if (searchDirection == 0)
                    zeroFork = ui32BitNumber; // Remember the fork
            }
        }
        // Save the bit
        if (searchDirection)
            *(current) |= 1 << ui32BitNumber % 8; // Set the bit in the current byte
        answerBit = (uint8_t) ((searchDirection == 0) ? WIRE_0 : WIRE_1); // Decide which one to turn off
        owSend(answerBit); // Disable interfering devices
        ui32BitNumber++; // Find the next bit
    } while (ui32BitNumber < 64); // Until the entire ROM with all bits is found
    ow->lastDiscrepancy = zeroFork; // Remember the fork
    for (; i < 7; i++)
        ow->lastROM[i] = ROM[i]; // Remember the last ROM
    return ow->lastDiscrepancy > 0;
}

// Returns the number of devices on the bus or an error code if less than 0
int owSearchCmd(OneWire *ow) {
    int device = 0, nextROM;
    owInit(ow);
    do {
        nextROM = hasNextRom(ow, (uint8_t*)(&ow->ids[device])); // Pass a pointer to the structure to store the next ROM
        if (nextROM < 0)
            return -1;
        device++;
    } while (nextROM && device < MAXDEVICES_ON_THE_BUS); // Keep searching as long as there are devices and they don't exceed the defined limit
    return device; // Return the ordinal number of the sensor (device) on the bus
}

void owSkipRomCmd(OneWire *ow) { // Sends the Skip ROM command, after this, the next command will be for all devices on the bus
    owResetCmd();
    owSendByte(ONEWIRE_SKIP_ROM);
}

void owMatchRomCmd(RomCode *rom) { // Allows the master to communicate with a specific slave device
    int i = 0;
    owResetCmd();
    owSendByte(ONEWIRE_MATCH_ROM); // Address a specific device
    for (; i < 8; i++)
        owSendByte(*(((uint8_t *) rom) + i)); // Iterate through the structure like an array to get the i-th byte
}

void owConvertTemperatureCmd(OneWire *ow, RomCode *rom) {
    owMatchRomCmd(rom); // Allows the master to communicate with a specific slave device
    owSendByte(ONEWIRE_CONVERT_TEMPERATURE); // Tell the sensor it's time to convert the temperature
}

/**
 * Method for reading scratchpad DS18B20 OR DS18S20
 * If the sensor is DS18B20, then data MUST be at least 9 bytes
 * If the sensor is DS18S20, then data MUST be at least 2 bytes
 * @param ow -- OneWire pointer
 * @param rom -- selected device on the bus
 * @param data -- buffer for data
 * @return data
 */
uint8_t *owReadScratchpadCmd(OneWire *ow, RomCode *rom, uint8_t *data) { // Read the sensor's memory
    uint16_t b = 0, p;
    switch (rom->family) {
        case DS18B20:
        case DS18S20:
            p = 72;  // 9*8 = 72, which corresponds to 9 bytes of data
            break;
        default:
            return data;
    }
    owMatchRomCmd(rom);
    owSendByte(ONEWIRE_READ_SCRATCHPAD); // Send the command to read the memory
    while (b < p) { // While we haven't processed all 9 bytes
        uint8_t pos = (uint8_t) ((p - 8) / 8 - (b / 8)); // Position of the byte being processed
        uint8_t bt;
        owSend(OW_READ);
        bt = owReadSlot(owEchoRead()); // Read the data
        if (bt == 1)
            data[pos] |= 1 << b % 8; // Set the bit in the correct position
        else
            data[pos] &= ~(1 << b % 8); // Reset the bit in the correct position
        b++; // Move to the next bit
    }
    return data;
}

void owWriteDS18B20Scratchpad(OneWire *ow, RomCode *rom, uint8_t th, uint8_t tl, uint8_t conf) {
    if (rom->family != DS18B20)
        return;
    owMatchRomCmd(rom); // Address a specific device
    owSendByte(ONEWIRE_WRITE_SCRATCHPAD); // Write to memory
    owSendByte(th); // Temperature thresholds
    owSendByte(tl);
    owSendByte(conf);
}

/**
 * Get the last measured temperature from DS18B20 or DS18S20. These temperature values MUST have been measured in previous operations.
 * If you want to measure a new value, you can set reSense to true. In this case, the next invocation of this method will return a value calculated in that step.
 * @param ow -- OneWire bus pointer
 * @param rom -- selected device
 * @param reSense -- Do you want to resense the temperature for the next time?
 * @return struct with data
 */
Temperature readTemperature(OneWire *ow, RomCode *rom, uint8_t reSense) {
    Scratchpad_DS18B20 *sp;
    Scratchpad_DS18S20 *spP;
    Temperature t;
    uint8_t pad[9];
    t.inCelsius = 0x00;
    t.frac = 0x00;
    sp = (Scratchpad_DS18B20 *) &pad;
    spP = (Scratchpad_DS18S20 *) &pad;
    switch (rom->family) {
        case DS18B20:
            owReadScratchpadCmd(ow, rom, pad); // Read memory for DS18B20
            t.inCelsius = (int8_t) (sp->temp_msb << 4) | (sp->temp_lsb >> 4); // Integer part
            t.frac = (uint8_t) ((((sp->temp_lsb & 0x0F)) * 10) >> 4); // Fractional part
            break;
        case DS18S20:
            owReadScratchpadCmd(ow, rom, pad); // Read memory for DS18S20
            t.inCelsius = spP->temp_lsb >> 1;
            t.frac = (uint8_t) 5 * (spP->temp_lsb & 0x01);
            break;
        default:
            return t;
    }
    if (reSense) {
        owConvertTemperatureCmd(ow, rom); // Immediately after retrieving the data, send a command to the sensor to convert the temperature
    }
    return t;
}

void owCopyScratchpadCmd(OneWire *ow, RomCode *rom) {
    owMatchRomCmd(rom);
    owSendByte(ONEWIRE_COPY_SCRATCHPAD);
}

void owRecallE2Cmd(OneWire *ow, RomCode *rom) {
    owMatchRomCmd(rom);
    owSendByte(ONEWIRE_RECALL_E2);
}

int get_ROMid(void) {
    if (owResetCmd() != ONEWIRE_NOBODY) { // Is anybody on the bus?
        devices = owSearchCmd(&ow); // Get ROM IDs of all devices on the bus or return an error code
        if (devices <= 0) {
            while (1) {
                pDelay = 1000000;
                for (i = 0; i < pDelay * 1; i++) /* Wait a bit. */
                    __asm__("nop");
            }
        }
        i = 0;
        for (; i < devices; i++) { // Print all found ROM IDs to the console
            RomCode *r = &ow.ids[i];
            uint8_t crc = owCRC8(r);
            crcOK = (crc == r->crc) ? "CRC OK" : "CRC ERROR!";
            devInfo.device = i;
            sprintf(devInfo.info, "SN: %02X/%02X%02X%02X%02X%02X%02X/%02X", r->family, r->code[5], r->code[4], r->code[3],
                    r->code[2], r->code[1], r->code[0], r->crc);
            if (crc != r->crc) {
                devInfo.device = i;
                sprintf(devInfo.info, "\n can't read cause CRC error");
            }
        }
    }
    pDelay = 1000000;
    for (i = 0; i < pDelay * 1; i++)
        __asm__("nop");
    if (strcmp(crcOK, "CRC OK") == 0) return 0;
    else return -1;
}

void get_Temperature(void) {
    i = 0;
    for (; i < devices; i++) {
        switch ((ow.ids[i]).family) { // Identify the type of sensor
            case DS18B20:
                // The value of the previous measurement will be returned!
                t = readTemperature(&ow, &ow.ids[i], 1);
                Temp[i] = (float) (t.inCelsius * 10 + t.frac) / 10.0;
                temperature = Temp[i];
                break;
            case DS18S20:
                t = readTemperature(&ow, &ow.ids[i], 1);
                Temp[i] = (float) (t.inCelsius * 10 + t.frac) / 10.0;
                temperature = Temp[i];
                break;
            case 0x00:
                break;
            default:
                // error handler
                break;
        }
    }
}
