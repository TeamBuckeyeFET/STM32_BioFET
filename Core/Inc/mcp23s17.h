/*
 * mcp23s17.h
 *
 *  Created on: Feb 19, 2026
 *      Author: BioFET Team
 *
 *  Simple polling driver for MCP23S17 SPI I/O Expander.
 */

#ifndef INC_MCP23S17_H_
#define INC_MCP23S17_H_

#include "stm32f4xx_hal.h"

// Registers (IOCON.BANK = 0)
#define MCP_IODIRA   0x00
#define MCP_IODIRB   0x01
#define MCP_GPIOA    0x12
#define MCP_GPIOB    0x13
#define MCP_OLATA    0x14
#define MCP_OLATB    0x15

// Structure to hold device context
typedef struct {
    SPI_HandleTypeDef *hspi;    // Pointer to SPI handle
    GPIO_TypeDef *cs_port;      // GPIO Port for Chip Select
    uint16_t cs_pin;            // GPIO Pin for Chip Select
    uint8_t device_addr;        // Hardware address (usually 0x40 if A0-A2 grounded)
    uint16_t current_output;    // Cache of current output state (16 bits for Port A + B)
} MCP23S17_Handle_t;

// Function Prototypes
void MCP_Init(MCP23S17_Handle_t *dev, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin, uint8_t addr);
void MCP_SetMode(MCP23S17_Handle_t *dev, uint16_t pin, uint8_t mode); // 1 = Input, 0 = Output
void MCP_WritePin(MCP23S17_Handle_t *dev, uint16_t pin, uint8_t state);
void MCP_TogglePin(MCP23S17_Handle_t *dev, uint16_t pin);
void MCP_WritePort(MCP23S17_Handle_t *dev, uint16_t val); // Write all 16 pins

#endif /* INC_MCP23S17_H_ */
