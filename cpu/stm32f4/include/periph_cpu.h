/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_stm32f4
 * @{
 *
 * @file
 * @brief           CPU specific definitions for internal peripheral handling
 *
 * @author          Hauke Petersen <hauke.peterse@fu-berlin.de>
 */

#ifndef CPU_PERIPH_H_
#define CPU_PERIPH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Define mandatory GPIO types for the STM32F4 MCU
 * @{
 */
typedef uint32_t gpio_t;
typedef uint8_t  gpio_mux_t;
/** @} */

/**
 * @brief   Mandatory macro for easy defining of GPIO pins
 */
#define GPIO(x, y)          ((GPIOA_BASE + (x << 10)) | y)

/**
 * @brief   Available ports on the STM32F4 family
 */
enum {
    PORT_A = 0,             /**< port A */
    PORT_B = 1,             /**< port B */
    PORT_C = 2,             /**< port C */
    PORT_D = 3,             /**< port D */
    PORT_E = 4,             /**< port E */
    PORT_F = 5,             /**< port F */
    PORT_G = 6,             /**< port G */
    PORT_H = 7,             /**< port H */
    PORT_I = 8              /**< port I */
};

/**
 * @brief   Available MUX values for configuring a pin's alternate function
 */
enum {
    GPIO_MUX_AF0 = 0,       /**< use alternate function 0 */
    GPIO_MUX_AF1,           /**< use alternate function 1 */
    GPIO_MUX_AF2,           /**< use alternate function 2 */
    GPIO_MUX_AF3,           /**< use alternate function 3 */
    GPIO_MUX_AF4,           /**< use alternate function 4 */
    GPIO_MUX_AF5,           /**< use alternate function 5 */
    GPIO_MUX_AF6,           /**< use alternate function 6 */
    GPIO_MUX_AF7,           /**< use alternate function 7 */
    GPIO_MUX_AF8,           /**< use alternate function 8 */
    GPIO_MUX_AF9,           /**< use alternate function 9 */
    GPIO_MUX_AF10,          /**< use alternate function 10 */
    GPIO_MUX_AF11,          /**< use alternate function 11 */
    GPIO_MUX_AF12,          /**< use alternate function 12 */
    GPIO_MUX_AF13,          /**< use alternate function 13 */
    GPIO_MUX_AF14           /**< use alternate function 14 */
};

#ifdef __cplusplus
}
#endif

#endif /* CPU_PERIPH_H_ */
/** @} */
