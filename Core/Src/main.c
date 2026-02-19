/*
 * main.c
 *
 *  Created on: Feb 19, 2026
 *      Author: BioFET Team
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mcp23s17.h"
#include "w25q32.h" // Flash Driver
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart1;

// Global Handles for the 3 Expanders
MCP23S17_Handle_t hExpander1; // FET 1 & 2
MCP23S17_Handle_t hExpander2; // FET 3 & 4
MCP23S17_Handle_t hExpander3; // Peripherals

// ==============================================================================
//  GLOBAL CONFIGURATION VARIABLES (Controlled via UART)
// ==============================================================================

uint8_t g_TestType = 2;            // Default to Ramping
float g_TestRunTimeMinutes = 5.0f; // Default 5 minutes
float g_ConstantDAC_HV = 5.0f;     // Default 5V
float g_ConstantDAC_LV = 0.5f;     // Default 0.5V
uint8_t g_TestRunning = 0;         // 0 = Idle, 1 = Running
uint32_t g_DataOffset = 0;         // Log Offset

// Startup / Hardware Config
#define BOOT_SWITCH_PIN GPIO_PIN_13
#define BOOT_SWITCH_PORT GPIOC

// FLASH CONFIG
// Define the CS Pin for the Flash here (or in main.h)
// User to confirmation pin: Assuming PB0 for now as placeholder
#define FLASH_CS_Pin GPIO_PIN_0
#define FLASH_CS_GPIO_Port GPIOB

// UART Reception Buffer
#define UART_RX_BUFFER_SIZE 64
char rx_buffer[UART_RX_BUFFER_SIZE];
uint8_t rx_index = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void Expander_Init(void);
void ProcessCommand(char *cmd);
void SendResponse(const char *msg);

// DAC Helper Prototypes
void DAC_SetVoltage_0_10V(float voltage);
void DAC_SetVoltage_N1_1V(float voltage);
void OffloadMemory(void);
void SaveConfig(void);
void LoadConfig(void);
void ClearFlash(void);

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();

  /* Initialize the SPI Expanders */
  Expander_Init();

  // Load Saved Settings from Flash (Stub)
  LoadConfig();

  // -------------------------------------------------------------------------
  // OFFLINE MODE CHECK
  // -------------------------------------------------------------------------
  // Check if Boot Switch is Active (Assume Active LOW or HIGH depending on
  // hardware) For standard "User Button" on STM32 (blue button), it's usually
  // Active LOW or HIGH. We'll assume Active LOW for a switch connected to
  // Ground, or HIGH if VCC. Let's check state. If using on-board button (PC13
  // on BlackPill), it's active LOW.
  if (HAL_GPIO_ReadPin(BOOT_SWITCH_PORT, BOOT_SWITCH_PIN) == GPIO_PIN_RESET) {
    // Button Pressed / Switch Active -> Auto Start
    g_TestRunning = 1;
    // No UART message here, as we might not be connected to PC
  } else {
    SendResponse("BioFET Ready\n");
  }

  uint32_t start_tick = 0;

  while (1) {
    // -----------------------------------------------------------------------
    // 1. UART COMMAND PROCESSING
    // -----------------------------------------------------------------------
    uint8_t rx_byte;
    if (HAL_UART_Receive(&huart1, &rx_byte, 1, 0) == HAL_OK) {
      if (rx_byte == '\n' || rx_byte == '\r') {
        if (rx_index > 0) {
          rx_buffer[rx_index] = '\0'; // Null terminate
          ProcessCommand(rx_buffer);
          rx_index = 0;
        }
      } else {
        if (rx_index < UART_RX_BUFFER_SIZE - 1) {
          rx_buffer[rx_index++] = rx_byte;
        } else {
          // Buffer overflow, reset
          rx_index = 0;
        }
      }
    }

    // -----------------------------------------------------------------------
    // 2. TEST LOGIC
    // -----------------------------------------------------------------------
    if (g_TestRunning) {
      if (start_tick == 0) {
        start_tick = HAL_GetTick(); // First run init
        // Erase Data Sector 1 (or clearer: The Offload function expects raw
        // data) For this demo, let's just overwrite. In real app, maybe erase
        // first? W25Q_EraseSector(DATA_ADDR_START); // Optional: Clear data log
        // area on start
      }

      uint32_t current_tick = HAL_GetTick();

      // --- DATA LOGGING ---
      static uint32_t last_log_tick = 0;

      if (current_tick - last_log_tick >= 100) { // Log every 100ms
        last_log_tick = current_tick;

        // Format: Timestamp(ms), Voltage(V), Current(uA) -> Binary or Text?
        // Text CSV is easier for direct offload. "100,5.0,1.2\n"
        char log_buf[32];
        float sim_voltage = 0.0f;
        if (g_TestType == 2) {
          // Calc sim voltage
          float duration_ms = g_TestRunTimeMinutes * 60.0f * 1000.0f;
          float elapsed = (float)(current_tick - start_tick);
          sim_voltage = (elapsed / duration_ms) * 10.0f;
        } else {
          sim_voltage = g_ConstantDAC_HV;
        }

        int len =
            sprintf(log_buf, "%lu,%.2f,%.2f\n", (current_tick - start_tick),
                    sim_voltage, sim_voltage * 0.5f); // Dummy Current

        W25Q_SaveData(g_DataOffset, (uint8_t *)log_buf, len);
        g_DataOffset += len;
      }

      if (g_TestType == 1) {
        // --- CONSTANT VOLTAGE ---
        DAC_SetVoltage_0_10V(g_ConstantDAC_HV);
        DAC_SetVoltage_N1_1V(g_ConstantDAC_LV);
      } else if (g_TestType == 2) {
        // --- RAMPING VOLTAGE ---
        float duration_ms = g_TestRunTimeMinutes * 60.0f * 1000.0f;
        float elapsed_ms = (float)(current_tick - start_tick);

        if (elapsed_ms >= duration_ms) {
          // Auto-stop
          g_TestRunning = 0;
          SendResponse("TEST_COMPLETE\n");
          DAC_SetVoltage_0_10V(0.0f);
          start_tick = 0;
          data_offset = 0; // Reset logging offset? Or keep it?
          // For this simple demo, we reset start command logic but maybe offset
          // persists until offload? Left as is: Next START will overwrite from
          // offset 0 (implied by logic, though static var persists)
          // FIXME: static data_offset should be reset on START command.
        } else {
          float progress = elapsed_ms / duration_ms;
          float current_voltage = progress * 10.0f;
          DAC_SetVoltage_0_10V(current_voltage);
          DAC_SetVoltage_N1_1V(0.0f);
        }
      }
    } else {
      // Idle
      start_tick = 0;
    }

    // Small delay to prevent tight loop from starving UART if using polling
    // mostly But since UART is polled with 0 timeout, we don't want a large
    // delay here. However, for the visual ramp, 10ms is fine.
    HAL_Delay(10);
  }
}

void ProcessCommand(char *cmd) {
  // Simple command parser
  // CMD:ARG

  if (strncmp(cmd, "SET_TYPE", 8) == 0) {
    int type = atoi(cmd + 9); // Skip "SET_TYPE "
    if (type == 1 || type == 2) {
      g_TestType = type;
      SendResponse("OK: Type Set\n");
    } else {
      SendResponse("ERR: Invalid Type\n");
    }
  } else if (strncmp(cmd, "SET_TIME", 8) == 0) {
    float time = atof(cmd + 9);
    if (time > 0) {
      g_TestRunTimeMinutes = time;
      SendResponse("OK: Time Set\n");
    } else {
      SendResponse("ERR: Invalid Time\n");
    }
  } else if (strncmp(cmd, "SAVE_CONFIG", 11) == 0) {
    SaveConfig();
    SendResponse("OK: Config Saved\n");
  } else if (strncmp(cmd, "CLEAR_FLASH", 11) == 0) {
    ClearFlash();
    SendResponse("OK: Flash Cleared\n");
  } else if (strncmp(cmd, "START", 5) == 0) {
    g_TestRunning = 1;
    g_DataOffset = 0; // Reset Log
    SendResponse("OK: Started\n");
  } else if (strncmp(cmd, "STOP", 4) == 0) {
    g_TestRunning = 0;
    DAC_SetVoltage_0_10V(0.0f); // Safety Reset
    SendResponse("OK: Stopped\n");
  } else if (strncmp(cmd, "READ_FLASH", 10) == 0) {
    OffloadMemory();
  } else if (strncmp(cmd, "PING", 4) == 0) {
    SendResponse("PONG\n");
  } else {
    SendResponse("ERR: Unknown Command\n");
  }
}

void SendResponse(const char *msg) {
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 100);
}

void SaveConfig(void) {
  BioFET_Config_t cfg;
  cfg.TestType = g_TestType;
  cfg.RunTimeMinutes = g_TestRunTimeMinutes;
  cfg.MagicNumber = 0xB10FE701;

  W25Q_SaveConfig(&cfg);
}

void LoadConfig(void) {
  BioFET_Config_t cfg;
  if (W25Q_LoadConfig(&cfg) == 1) {
    // Valid Config Found
    g_TestType = cfg.TestType;
    g_TestRunTimeMinutes = cfg.RunTimeMinutes;
  } else {
    // Invalid or Empty, use defaults
    g_TestType = 2;
    g_TestRunTimeMinutes = 5.0f;
  }
}

void ClearFlash(void) {
  // Erase chip (Takes a while!)
  // Or just erase Data Sectors? Chip erase is simplest for "Clear All"
  W25Q_EraseChip();
}

void OffloadMemory(void) {
  SendResponse("BEGIN_DATA\n");

  // Read stored data from Flash
  // We know g_DataOffset is the end of data (assuming it wasn't reset by
  // reboot) If rebooted, g_DataOffset is 0. We might need to store "Data
  // Length" in Config Sector? For now, let's assume we read until 0xFF or some
  // limit. Better: For this demo, we only offload if we have g_DataOffset > 0,
  // same session. OR we scan.

  // Simple approach: Read 4KB chunks and send
  // Since we don't have stored length after reboot, reading until 0xFF is risky
  // (uninitialized flash). Let's just read first 4KB for demo or use
  // g_DataOffset if valid.

  uint32_t len_to_read = g_DataOffset;
  if (len_to_read == 0)
    len_to_read = 1024; // Fallback to 1KB dump logic if 0

  uint8_t buf[64];
  uint32_t read_ptr = 0;

  while (read_ptr < len_to_read) {
    W25Q_Read(buf, DATA_ADDR_START + read_ptr, 64);
    // Ensure null term or send as bytes?
    // Our data is text "100,5.0...\n".
    // We can just pipe it out.
    // Be careful with newlines.

    HAL_UART_Transmit(&huart1, buf, 64, 100);
    read_ptr += 64;
  }

  SendResponse("\nEND_DATA\n");
}
/*
 * DAC CONTROL FUNCTIONS (STUBS)
 * USER: You need to implement the actual SPI commands here based on your DAC
 * part number.
 */

void DAC_SetVoltage_0_10V(float voltage) {
  // 1. Enable DAC CS (Expander 3, Pin 0 for DAC 0-10V)
  MCP_WritePin(&hExpander3, EXP3_DAC_0_10V_CS_PIN, GPIO_PIN_RESET);

  // 2. Send SPI Command to DAC
  // Example for 12-bit DAC:
  // uint16_t code = (uint16_t)((voltage / 10.0f) * 4095.0f);
  // uint8_t data[2] = { (code >> 8) & 0xFF, code & 0xFF };
  // HAL_SPI_Transmit(&hspi1, data, 2, 100);

  // 3. Disable DAC CS
  MCP_WritePin(&hExpander3, EXP3_DAC_0_10V_CS_PIN, GPIO_PIN_SET);
}

void DAC_SetVoltage_N1_1V(float voltage) {
  // 1. Enable DAC CS (Expander 3, Pin 1 for DAC -1 to 1V)
  MCP_WritePin(&hExpander3, EXP3_DAC_N1_1V_CS_PIN, GPIO_PIN_RESET);

  // 2. Send SPI Command implementation ...

  // 3. Disable DAC CS
  MCP_WritePin(&hExpander3, EXP3_DAC_N1_1V_CS_PIN, GPIO_PIN_SET);
}

/**
 * @brief System Clock Configuration
 * @retval None
 * @note This is a generic configuration for STM32F401.
 *       If your board has a crystal, you might need to enable HSE.
 *       Here we use HSI (16MHz) for safety/simplicity.
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState =
      RCC_PLL_NONE; // Run directly from HSI for now
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void) {
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT; // Software Slave Select (managed by GPIOs)
  hspi1.Init.BaudRatePrescaler =
      SPI_BAUDRATEPRESCALER_16; // Adjust speed as needed
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE(); // Enabled for PC13 (Boot Switch)

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, EXP1_CS_Pin | EXP2_CS_Pin | EXP3_CS_Pin,
                    GPIO_PIN_SET); // Default CS High (Inactive)

  /*Configure GPIO pins : EXP1_CS_Pin EXP2_CS_Pin EXP3_CS_Pin */
  GPIO_InitStruct.Pin = EXP1_CS_Pin | EXP2_CS_Pin | EXP3_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PC13 (Boot Switch) */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : FLASH_CS (PB0) */
  HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin,
                    GPIO_PIN_SET); // Default High (Inactive)
  GPIO_InitStruct.Pin = FLASH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(FLASH_CS_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief Initialize the 3 MCP23S17 Expanders
 */
static void Expander_Init(void) {
  // Expander 1: FET 1 & 2
  MCP_Init(&hExpander1, &hspi1, EXP1_CS_GPIO_Port, EXP1_CS_Pin, 0x40);

  // Expander 2: FET 3 & 4
  MCP_Init(&hExpander2, &hspi1, EXP2_CS_GPIO_Port, EXP2_CS_Pin, 0x40);

  // Expander 3: Peripherals
  MCP_Init(&hExpander3, &hspi1, EXP3_CS_GPIO_Port, EXP3_CS_Pin, 0x40);
}

void Error_Handler(void) {
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
}
