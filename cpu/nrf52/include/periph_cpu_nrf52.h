/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_nrf52
 * @{
 *
 * @file
 * @brief           NRF52 specific peripheral configuration
 *
 * @author          Hauke Petersen <hauke.peterse@fu-berlin.de>
 */

#ifndef PERIPH_CPU_NRF52_H
#define PERIPH_CPU_NRF52_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Peripheral clock speed (fixed to 16MHz for NRF52 based CPUs)
 */
#define PERIPH_CLOCK        (16000000U)

/**
 * @brief   The PWM unit on the NRF52 supports 4 channels per device
 */
#define PWM_CHANNELS        (4U)

/**
 * @brief   Generate PWM mode values
 *
 * To encode the PWM mode, we use two bit:
 * - bit  0: select up or up-and-down counting
 * - bit 15: select polarity
 */
#define PWM_MODE(ud, pol)   (ud | (pol << 15))

/**
 * @brief   Override the PWM mode definitions
 * @{
 */
#define HAVE_PWM_MODE_T
typedef enum {
    PWM_LEFT       = PWM_MODE(0, 1),    /**< left aligned PWM */
    PWM_RIGHT      = PWM_MODE(0, 0),    /**< right aligned PWM */
    PWM_CENTER     = PWM_MODE(1, 1),    /**< center aligned PWM */
    PWM_CENTER_INV = PWM_MODE(1, 0)     /**< center aligned with negative polarity */
} pwm_mode_t;
/** @} */

/**
 * @brief   PWM configuration options
 *
 * Each device supports up to 4 channels. If you want to use less than 4
 * channels, just set the unused pins to GPIO_UNDEF.
 *
 * @note    define unused pins only from right to left, so the defined channels
 *          always start with channel 0 to x and the undefined ones are from x+1
 *          to PWM_CHANNELS.
 */
typedef struct {
    NRF_PWM_Type *dev;
    uint32_t pin[PWM_CHANNELS];
} pwm_conf_t;


#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CPU_NRF52_H */
/** @} */
