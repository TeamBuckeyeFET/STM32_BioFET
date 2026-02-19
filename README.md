# STM32 BioFET Firmware

This project is a firmware implementation for the STM32F401CCU6 BioFET controller. It communicates with 3x MCP23S17 SPI I/O Expanders to control various analog peripherals and FETs.

## Project Structure
The code is designed to be dropped into a standard STM32CubeIDE project.

1.  **Create a New Project** in STM32CubeIDE.
    *   Target: **STM32F401CCU6**
    *   Initialize peripherals: **No** (Empty project is better, or clear the default code)
2.  **Copy the `Core` folder** from this repository into your new project, replacing the existing `Core` folder.
3.  **Build** the project.

## Pin Configuration (USER TODO)
Open `Core/Inc/main.h`. This file contains all the pin definitions.
**You MUST verify the Chip Select (CS) pins** for the 3 SPI expanders:

```c
// Expander 1 (FET 1 & 2 Control)
#define EXP1_CS_Pin         GPIO_PIN_0
#define EXP1_CS_GPIO_Port   GPIOB  // <--- Verify this!

// Expander 2 (FET 3 & 4 Control)
#define EXP2_CS_Pin         GPIO_PIN_1
#define EXP2_CS_GPIO_Port   GPIOB  // <--- Verify this!

// Expander 3 (Peripherals)
#define EXP3_CS_Pin         GPIO_PIN_2
#define EXP3_CS_GPIO_Port   GPIOB  // <--- Verify this!
```

## User Configuration & Test Modes
Open `Core/Src/main.c` and look at the top section marked `USER CONFIGURATION SECTION`.

### 1. Select Test Type
Change the `TEST_TYPE` definition:
*   `1`: **Constant Voltage Mode**. Sets the DACs to fixed voltages and holds them.
*   `2`: **Ramping Mode**. Ramps the 0-10V DAC from 0V to Max over a set time.

### 2. Configure Settings
*   **Test 2 Duration**: Change `TEST_RUN_TIME_MINUTES` (e.g., `5.0f` for 5 mins, `10.0f` for 10 mins).
*   **Test 1 Voltages**: Change `CONSTANT_DAC_HV_TARGET` and `CONSTANT_DAC_LV_TARGET`.

## How to Control Devices
The system uses the `MCP23S17_Handle_t` structures defined in `main.c` to control the expanders.

### Example: Enable ADC1
ADC1 is on Expander 3, Pin 2 (GPA2).
```c
// In main.c loop or function:
MCP_WritePin(&hExpander3, EXP3_ADC1_CS_PIN, GPIO_PIN_RESET); // Enable (Active Low)
// ... Perform SPI data reading ...
MCP_WritePin(&hExpander3, EXP3_ADC1_CS_PIN, GPIO_PIN_SET);   // Disable
```

### Example: Set FET1 Gain Bit 0
FET1 Gain Bit 0 is on Expander 1, Pin 0 (GPA0).
```c
MCP_WritePin(&hExpander1, EXP1_FET1_GAIN_BIT0_PIN, GPIO_PIN_SET); // Set High
```

## Hardware Assumptions
1.  **SPI Bus**: STM32 SPI1 (PA5/SCK, PA6/MISO, PA7/MOSI) is connected to ALL expanders.
2.  **CS Lines**: Each expander has a unique CS line committed to it.
3.  **Addresses**: The driver assumes address `0x40` for all expanders (A0/A1/A2 pins grounded). If your hardware uses addressing (e.g., all 3 expanders share ONE CS line but have different addresses), change the `MCP_Init` call in `main.c`.
