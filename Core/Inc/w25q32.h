/*
 * w25q32.h
 *
 *  Created on: Feb 19, 2026
 *      Author: BioFET Team
 */

#ifndef INC_W25Q32_H_
#define INC_W25Q32_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// ============================================================================
// CONFIGURATION
// ============================================================================
// ASSUMPTION: Flash is on same SPI as Expanders (hspi1) or separate?
// User said "designate a pin", implying generic GPIO CS.
extern SPI_HandleTypeDef hspi1;
#define W25Q_SPI_HANDLE &hspi1

// DEFINE THIS PIN IN MAIN.H or IOC
// For now, we define default placeholders. Valid pin must be set in main.c
// MX_GPIO_Init
#ifndef FLASH_CS_GPIO_Port
#define FLASH_CS_GPIO_Port GPIOB
#endif
#ifndef FLASH_CS_Pin
#define FLASH_CS_Pin GPIO_PIN_0 // Example placeholder
#endif

// ============================================================================
// MEMORY MAP
// ============================================================================
// W25Q32JV: 32Mbit = 4MByte
// 64 Blocks of 64KB
// 1024 Sectors of 4KB
// Pages of 256 Bytes

#define FLASH_SECTOR_SIZE 4096
#define FLASH_PAGE_SIZE 256

// CONFIG_SECTOR: Sector 0 for storing Settings (Type, Duration)
#define CONFIG_ADDR_START 0x000000

// DATA_START_SECTOR: Sector 1 start for Test Data
#define DATA_ADDR_START 0x001000

// ============================================================================
// COMMANDS
// ============================================================================
#define CMD_WRITE_ENABLE 0x06
#define CMD_WRITE_DISABLE 0x04
#define CMD_READ_STATUS_1 0x05
#define CMD_READ_DATA 0x03
#define CMD_PAGE_PROGRAM 0x02
#define CMD_SECTOR_ERASE 0x20 // 4KB
#define CMD_BLOCK_ERASE 0xD8  // 64KB
#define CMD_CHIP_ERASE 0xC7
#define CMD_JEDEC_ID 0x9F

// ============================================================================
// DATA STRUCTURES
// ============================================================================
typedef struct {
  uint8_t TestType;
  float RunTimeMinutes;
  // Add padding/magic number to verify validity
  uint32_t MagicNumber; // 0xB10FE701 (BioFET01)
} BioFET_Config_t;

// ============================================================================
// FUNCTIONS
// ============================================================================
void W25Q_Reset(void);
uint32_t W25Q_ReadID(void);
void W25Q_EraseSector(uint32_t address);
void W25Q_EraseChip(void);
void W25Q_Write(uint8_t *pData, uint32_t writeAddr, uint32_t size);
void W25Q_Read(uint8_t *pBuffer, uint32_t readAddr, uint32_t size);

// Helpers
void W25Q_SaveConfig(BioFET_Config_t *cfg);
uint8_t W25Q_LoadConfig(BioFET_Config_t *cfg); // Returns 1 if valid, 0 if empty
void W25Q_SaveData(uint32_t offset, uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* INC_W25Q32_H_ */
