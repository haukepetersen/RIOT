/*
 * Copyright (C) 2016 Leon George
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */
/**
 * @defgroup    boards_cc2650stk CC2650STK
 * @ingroup     boards
 * @brief       SimpleLink™ CC2650 sensor tag
 * @{
 *
 * @author      Leon M. George <leon@georgemail.eu>
 */

#ifndef BOARD_H_
#define BOARD_H_

#include "cpu.h"
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BUTTON1_DIO         GPIO_PIN(0,4)
#define BUTTON2_DIO         GPIO_PIN(0,0)


/**
 * @name macros for controlling the on-board LEDs.
 * @{
 */
#define LED_RED_DIO         GPIO_PIN(0,10)
#define LED_GREEN_DIO       GPIO_PIN(0,15)

#define LED_RED_ON          gpio_set(LED_RED_DIO)
#define LED_RED_OFF         gpio_clear(LED_RED_DIO)
#define LED_RED_TOGGLE      gpio_toggle(LED_RED_DIO)

#define LED_GREEN_ON        gpio_set(LED_GREEN_DIO)
#define LED_GREEN_OFF       gpio_clear(LED_GREEN_DIO)
#define LED_GREEN_TOGGLE    gpio_toggle(LED_GREEN_DIO)
/** @} */

/**
 * @brief initialize board specific hardware(LEDs)
 */
void board_init(void);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* BOARD_H_ */
/** @} */
