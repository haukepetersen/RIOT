/*
 * Copyright (C) 2015-2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_nrf5x_common
 * @{
 *
 * @file
 * @brief           CPU specific definitions for handling peripherals
 *
 * @author          Hauke Petersen <hauke.peterse@fu-berlin.de>
 */

#ifndef CPU_PERIPH_H
#define CPU_PERIPH_H

#include "cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Iron out some differences in register and IRQ channel naming between
 *          the different nRFx family members
 * @{
 */
#if defined(CPU_FAM_NRF51)
#define GPIO_BASE           (NRF_GPIO)
#define UART_IRQN           (UART0_IRQn)
#elif defined(CPU_FAM_NRF52)
#define GPIO_BASE           (NRF_P0)
#define UART_IRQN           (UARTE0_UART0_IRQn)
#else
#error "nrf5x_common: no valid value for CPU_FAM_XX defined"
#endif
/** @} */

/**
 * @brief   Length of the CPU_ID in octets
 */
#define CPUID_LEN           (8U)

/**
 * @brief   Override macro for defining GPIO pins
 *
 * The port definition is used (and zeroed) to suppress compiler warnings
 */
#define GPIO_PIN(x,y)       ((x & 0) | y)

/**
 * @brief   Use shared I2C driver functions
 * @{
 */
#define PERIPH_I2C_NEEDS_WRITE_BYTE
#define PERIPH_I2C_NEEDS_WRITE_REG
/** @} */

/**
 * @brief   Override GPIO pull register select values
 * @{
 */
#define HAVE_GPIO_PP_T
typedef enum {
    GPIO_NOPULL = 0,        /**< do not use internal pull resistors */
    GPIO_PULLUP = 3,        /**< enable internal pull-up resistor */
    GPIO_PULLDOWN = 1       /**< enable internal pull-down resistor */
} gpio_pp_t;
/** @} */

/**
 * @brief   Override GPIO active flank values
 * @{
 */
#define HAVE_GPIO_FLANK_T
typedef enum {
    GPIO_FALLING = 2,       /**< emit interrupt on falling flank */
    GPIO_RISING  = 1,       /**< emit interrupt on rising flank */
    GPIO_BOTH    = 3        /**< emit interrupt on both flanks */
} gpio_flank_t;
/** @} */

/**
 * @brief   Override I2C speed values
 */
#define HAVE_I2C_SPEED_T
typedef enum {
    I2C_SPEED_LOW       = 1,      /**< low speed mode:    ~10kbit/s */
    I2C_SPEED_NORMAL    = TWI_FREQUENCY_FREQUENCY_K100,       /**< normal mode:       ~100kbit/s */
    I2C_SPEED_FAST      = TWI_FREQUENCY_FREQUENCY_K250,         /**< fast mode:         ~400kbit/sj */
    I2C_SPEED_FAST_PLUS = TWI_FREQUENCY_FREQUENCY_K400,    /**< fast plus mode:    ~1Mbit/s */
    I2C_SPEED_HIGH      = 2    /**< high speed mode:   ~3.4Mbit/s */
} i2c_speed_t;

/**
 * @brief   I2C configuration options
 */
typedef struct {
#if defined(CPU_FAM_NRF51)
    NRF_TWI_Type *dev;      /**< TWI master device */
#else
    NRF_TWIM_Type *dev;     /**< TWI master with easy DMA device */
#endif
    uint8_t sda_pin;        /**< SDA pin */
    uint8_t scl_pin;        /**< SCL pin */
} i2c_conf_t;

#ifdef __cplusplus
}
#endif

#endif /* CPU_PERIPH_H */
/** @} */
