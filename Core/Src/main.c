/*
 * main.c
 *
 *  Created on: Feb 19, 2026
 *      Author: BioFET Team
 */

#include "main.h"
#include "mcp23s17.h"

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart1;

// Global Handles for the 3 Expanders
MCP23S17_Handle_t hExpander1; // FET 1 & 2
MCP23S17_Handle_t hExpander2; // FET 3 & 4
MCP23S17_Handle_t hExpander3; // Peripherals

// ==============================================================================
//  USER CONFIGURATION SECTION
// ==============================================================================

/*
 * TEST TYPE SELECTION
 * 1 = Constant Voltage Mode (Sets DACs to fixed values)
 * 2 = Ramping Mode (Ramps 0-10V DAC from 0V to Max over time)
 */
#define TEST_TYPE 2

/*
 * TEST TYPE 2 SETTINGS (Ramping)
 * Set the test duration in MINUTES.
 * Examples:
 *   5.0f  = 5 Minutes
 *   10.0f = 10 Minutes
 */
#define TEST_RUN_TIME_MINUTES 5.0f

/*
 * TEST TYPE 1 SETTINGS (Constant Voltage)
 * Set the target voltages for the DACs.
 */
#define CONSTANT_DAC_HV_TARGET 5.0f // Target for 0-10V DAC (Volts)
#define CONSTANT_DAC_LV_TARGET 0.5f // Target for -1V to 1V DAC (Volts)

// ==============================================================================
//  END USER CONFIGURATION
// ==============================================================================

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void Expander_Init(void);

// DAC Helper Prototypes
void DAC_SetVoltage_0_10V(float voltage);
void DAC_SetVoltage_N1_1V(float voltage);

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

  // -------------------------------------------------------------------------
  // EXECUTE TEST LOGIC
  // -------------------------------------------------------------------------

  if (TEST_TYPE == 1) {
    // --- TEST TYPE 1: CONSTANT VOLTAGE ---
    // Sets the DACs to the constant values defined in USER CONFIGURATION and
    // stays there.

    DAC_SetVoltage_0_10V(CONSTANT_DAC_HV_TARGET);
    DAC_SetVoltage_N1_1V(CONSTANT_DAC_LV_TARGET);

    while (1) {
      // Loop forever, maintaining voltage.
      // Optional: Add logging or status LED capability here.
      HAL_Delay(1000);
    }
  } else if (TEST_TYPE == 2) {
    // --- TEST TYPE 2: RAMPING VOLTAGE ---
    // Ramps the 0-10V DAC from 0V up to 10V (or max) over
    // TEST_RUN_TIME_MINUTES.

    uint32_t start_tick = HAL_GetTick();
    float duration_ms = TEST_RUN_TIME_MINUTES * 60.0f * 1000.0f;

    while (1) {
      uint32_t current_tick = HAL_GetTick();
      float elapsed_ms = (float)(current_tick - start_tick);

      // Clamp elapsed time to duration to prevent overshoot if we want to stop
      // at max
      if (elapsed_ms > duration_ms) {
        elapsed_ms = duration_ms;
      }

      // Calculate progress (0.0 to 1.0)
      float progress = elapsed_ms / duration_ms;

      // Calculate Voltage (Linear Ramp: 0V to 10V)
      // Adjust 10.0f if the max voltage of the DAC is different or you want a
      // different slope.
      float current_voltage = progress * 10.0f;

      // Update DAC
      DAC_SetVoltage_0_10V(current_voltage);

      // Also set the secondary DAC to a baseline or modify logic if it should
      // also ramp
      DAC_SetVoltage_N1_1V(0.0f);

      HAL_Delay(100); // Update rate ~10Hz
    }
  } else {
    // Invalid Test Type Selected
    while (1) {
      // Blink Error or similar
      HAL_Delay(100);
    }
  }
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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, EXP1_CS_Pin | EXP2_CS_Pin | EXP3_CS_Pin,
                    GPIO_PIN_SET); // Default CS High (Inactive)

  /*Configure GPIO pins : EXP1_CS_Pin EXP2_CS_Pin EXP3_CS_Pin */
  GPIO_InitStruct.Pin = EXP1_CS_Pin | EXP2_CS_Pin | EXP3_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
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
