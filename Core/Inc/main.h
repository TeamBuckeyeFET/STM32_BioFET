/*
 * main.h
 *
 *  Created on: Feb 19, 2026
 *      Author: BioFET Team
 */

#ifndef INC_MAIN_H_
#define INC_MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/

// =============================================================================
// STM32 NATIVE PIN DEFINITIONS
// Ref: STM32F401CCU6 Pinout
// =============================================================================

/* 
 * SPI1 CONNECTIONS (Shared Bus for all 3 Expanders + Memory?)
 * NOTE: Check if Flash Memory shares this bus or needs SPI2.
 * Current assumption: All devices on SPI1.
 */
#define SPI1_SCK_Pin        GPIO_PIN_5
#define SPI1_SCK_GPIO_Port  GPIOA
#define SPI1_MISO_Pin       GPIO_PIN_6
#define SPI1_MISO_GPIO_Port GPIOA
#define SPI1_MOSI_Pin       GPIO_PIN_7
#define SPI1_MOSI_GPIO_Port GPIOA

/*
 * CHIP SELECTS for SPI DEVICES (STM32 GPIOs)
 * USER: Verify these pins are free and physically connected to the CS pin of the respective device.
 */
// Expander 1 (FET 1 & 2 Control)
#define EXP1_CS_Pin         GPIO_PIN_0
#define EXP1_CS_GPIO_Port   GPIOB  // USER: Change this if connected elsewhere

// Expander 2 (FET 3 & 4 Control)
#define EXP2_CS_Pin         GPIO_PIN_1
#define EXP2_CS_GPIO_Port   GPIOB  // USER: Change this if connected elsewhere

// Expander 3 (Peripherals: ADC, DAC, Flash, ESP32)
#define EXP3_CS_Pin         GPIO_PIN_2
#define EXP3_CS_GPIO_Port   GPIOB  // USER: Change this if connected elsewhere

/*
 * DEBUG / CONSOLE (USART1)
 */
#define DEBUG_TX_Pin        GPIO_PIN_9
#define DEBUG_TX_GPIO_Port  GPIOA
#define DEBUG_RX_Pin        GPIO_PIN_10
#define DEBUG_RX_GPIO_Port  GPIOA

/*
 * SWD DEBUG (Standard)
 */
#define SWDIO_Pin           GPIO_PIN_13
#define SWDIO_GPIO_Port     GPIOA
#define SWCLK_Pin           GPIO_PIN_14
#define SWCLK_GPIO_Port     GPIOA


// =============================================================================
// VIRTUAL PIN DEFINITIONS (MAPPED TO MCP23S17 EXPANDERS)
// These are NOT STM32 pins. They are 0-15 indices on the expander ports.
// Port A = 0-7, Port B = 8-15
// =============================================================================

/*
 * EXPANDER 3: PERIPHERALS
 * Controls CS lines for other SPI devices
 */
#define EXP3_DAC_0_10V_CS_PIN      0  // GPA0
#define EXP3_DAC_N1_1V_CS_PIN      1  // GPA1
#define EXP3_ADC1_CS_PIN           2  // GPA2
#define EXP3_ADC2_CS_PIN           3  // GPA3
#define EXP3_ADC3_CS_PIN           4  // GPA4
#define EXP3_ADC4_CS_PIN           5  // GPA5
#define EXP3_FLASH_CS_PIN          6  // GPA6
#define EXP3_ESP32_CS_PIN          7  // GPA7

/*
 * EXPANDER 1: FET 1 & 2
 * Gain and Shunt controls
 */
// Fet 1
#define EXP1_FET1_GAIN_BIT0_PIN    0  // GPA0
#define EXP1_FET1_GAIN_BIT1_PIN    1  // GPA1
#define EXP1_FET1_GAIN_BIT2_PIN    2  // GPA2
#define EXP1_FET1_SHUNT_BIT0_PIN   3  // GPA3
#define EXP1_FET1_SHUNT_BIT1_PIN   4  // GPA4
#define EXP1_FET1_SHUNT_BIT2_PIN   5  // GPA5

// Fet 2
#define EXP1_FET2_GAIN_BIT0_PIN    8  // GPB0 (Note: Port B starts at index 8 in our driver abstraction)
#define EXP1_FET2_GAIN_BIT1_PIN    9  // GPB1
#define EXP1_FET2_GAIN_BIT2_PIN    10 // GPB2
#define EXP1_FET2_SHUNT_BIT0_PIN   11 // GPB3
#define EXP1_FET2_SHUNT_BIT1_PIN   12 // GPB4
#define EXP1_FET2_SHUNT_BIT2_PIN   13 // GPB5

/*
 * EXPANDER 2: FET 3 & 4
 * Gain and Shunt controls
 */
// Fet 3
#define EXP2_FET3_GAIN_BIT0_PIN    0  // GPA0
#define EXP2_FET3_GAIN_BIT1_PIN    1  // GPA1
#define EXP2_FET3_GAIN_BIT2_PIN    2  // GPA2
#define EXP2_FET3_SHUNT_BIT0_PIN   3  // GPA3
#define EXP2_FET3_SHUNT_BIT1_PIN   4  // GPA4
#define EXP2_FET3_SHUNT_BIT2_PIN   5  // GPA5

// Fet 4
#define EXP2_FET4_GAIN_BIT0_PIN    8  // GPB0
#define EXP2_FET4_GAIN_BIT1_PIN    9  // GPB1
#define EXP2_FET4_GAIN_BIT2_PIN    10 // GPB2
#define EXP2_FET4_SHUNT_BIT0_PIN   11 // GPB3
#define EXP2_FET4_SHUNT_BIT1_PIN   12 // GPB4
#define EXP2_FET4_SHUNT_BIT2_PIN   13 // GPB5

#ifdef __cplusplus
}
#endif

#endif /* INC_MAIN_H_ */
