/*
 * mcp23s17.c
 *
 *  Created on: Feb 19, 2026
 *      Author: BioFET Team
 */

#include "mcp23s17.h"

/*
 * Initializes the MCP23S17 instance.
 * Configures all pins as OUTPUT by default for this project (based on
 * requirements).
 */
void MCP_Init(MCP23S17_Handle_t *dev, SPI_HandleTypeDef *hspi,
              GPIO_TypeDef *cs_port, uint16_t cs_pin, uint8_t addr) {
  dev->hspi = hspi;
  dev->cs_port = cs_port;
  dev->cs_pin = cs_pin;
  dev->device_addr = addr; // Base write address (e.g. 0x40)
  dev->current_output = 0x0000;

  // Deselect initially
  HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);

  // Configure IODIRA and IODIRB to all OUTPUTS (0x00)
  // Opcode = addr | 0x00 (Write)
  // Register Address
  // Data

  uint8_t data[3];

  // Set Port A to Output
  data[0] = dev->device_addr; // Write
  data[1] = MCP_IODIRA;
  data[2] = 0x00; // All Output

  HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(dev->hspi, data, 3, 100);
  HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);

  // Set Port B to Output
  data[1] = MCP_IODIRB;
  data[2] = 0x00; // All Output

  HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(dev->hspi, data, 3, 100);
  HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
}

void MCP_WritePin(MCP23S17_Handle_t *dev, uint16_t pin, uint8_t state) {
  if (state) {
    dev->current_output |= (1 << pin);
  } else {
    dev->current_output &= ~(1 << pin);
  }
  MCP_WritePort(dev, dev->current_output);
}

void MCP_TogglePin(MCP23S17_Handle_t *dev, uint16_t pin) {
  dev->current_output ^= (1 << pin);
  MCP_WritePort(dev, dev->current_output);
}

/*
 * Writes the cached 16-bit value to OLATA and OLATB
 */
void MCP_WritePort(MCP23S17_Handle_t *dev, uint16_t val) {
  uint8_t data[3];

  // Write Port A (Low Byte)
  data[0] = dev->device_addr;
  data[1] = MCP_GPIOA; // Address for GPIOA
  data[2] = (uint8_t)(val & 0xFF);

  HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(dev->hspi, data, 3, 100);
  HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);

  // Write Port B (High Byte)
  data[1] = MCP_GPIOB; // Address for GPIOB
  data[2] = (uint8_t)((val >> 8) & 0xFF);

  HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(dev->hspi, data, 3, 100);
  HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
}
