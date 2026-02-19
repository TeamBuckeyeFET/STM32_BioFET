/*
 * stm32f4xx_hal_conf.h
 *
 * Minimal configuration for STM32F4xx HAL
 */

#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* ########################## Module Selection ############################## */
#define HAL_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED

/* ########################## Oscillator Values adaptation
 * ####################*/
#if !defined(HSE_VALUE)
#define HSE_VALUE                                                              \
  ((uint32_t)25000000) /*!< Value of the External oscillator in Hz */
#endif                 /* HSE_VALUE */

#if !defined(HSI_VALUE)
#define HSI_VALUE                                                              \
  ((uint32_t)16000000) /*!< Value of the Internal oscillator in Hz*/
#endif                 /* HSI_VALUE */

#if !defined(LSE_VALUE)
#define LSE_VALUE                                                              \
  ((uint32_t)32768) /*!< Value of the External Low Speed oscillator in Hz */
#endif              /* LSE_VALUE */

#if !defined(LSI_VALUE)
#define LSI_VALUE                                                              \
  ((uint32_t)32000) /*!< Value of the Internal Low Speed oscillator in Hz */
#endif              /* LSI_VALUE */

/* ########################## SysTick Timer Selection ########################*/
#define TICK_INT_PRIORITY ((uint32_t)0) /*!< tick interrupt priority */

/* ########################## Register Callback feature ##################### */
#define USE_HAL_SPI_REGISTER_CALLBACKS 0U
#define USE_HAL_UART_REGISTER_CALLBACKS 0U

/* Includes ------------------------------------------------------------------*/
#ifdef HAL_RCC_MODULE_ENABLED
#include "stm32f4xx_hal_rcc.h"
#endif /* HAL_RCC_MODULE_ENABLED */

#ifdef HAL_GPIO_MODULE_ENABLED
#include "stm32f4xx_hal_gpio.h"
#endif /* HAL_GPIO_MODULE_ENABLED */

#ifdef HAL_SPI_MODULE_ENABLED
#include "stm32f4xx_hal_spi.h"
#endif /* HAL_SPI_MODULE_ENABLED */

#ifdef HAL_UART_MODULE_ENABLED
#include "stm32f4xx_hal_uart.h"
#endif /* HAL_UART_MODULE_ENABLED */

#ifdef HAL_FLASH_MODULE_ENABLED
#include "stm32f4xx_hal_flash.h"
#endif /* HAL_FLASH_MODULE_ENABLED */

#ifdef HAL_PWR_MODULE_ENABLED
#include "stm32f4xx_hal_pwr.h"
#endif /* HAL_PWR_MODULE_ENABLED */

#ifdef HAL_CORTEX_MODULE_ENABLED
#include "stm32f4xx_hal_cortex.h"
#endif /* HAL_CORTEX_MODULE_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_HAL_CONF_H */
