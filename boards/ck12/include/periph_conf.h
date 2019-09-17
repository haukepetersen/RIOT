/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_ck12
 * @{
 *
 * @file
 * @brief       Peripheral configuration for the CK12 smart watch
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 */

#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#include "periph_cpu.h"
#include "cfg_clock_32_1.h"
#include "cfg_rtt_default.h"
#include "cfg_timer_default.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    UART configuration
 * @{
 */
// #define UART_NUMOF          (1U)
// #define UART_PIN_RX         GPIO_PIN(0, 2)
// #define UART_PIN_TX         GPIO_PIN(0, 3)
/** @} */

/**
 * @name    I2C configuration
 * @{
 */
static const i2c_conf_t i2c_config[] = {
    {
        .dev = NRF_TWIM0,
        .scl = 5,
        .sda = 3,
        .speed = I2C_SPEED_FAST_PLUS,
    },
    {
        .dev = NRF_TWIM1,
        .scl = 25,
        .sda = 26,
        .speed = I2C_SPEED_NORMAL,
    },
};


#define I2C_NUMOF           (sizeof(i2c_config) / sizeof(i2c_config[0]))
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
/** @} */