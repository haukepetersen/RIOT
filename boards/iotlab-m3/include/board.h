/*
 * Copyright (C) 2014-2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    boards_iotlab-m3 IoT-LAB M3 open node
 * @ingroup     boards
 * @brief       Board specific files for the iotlab-m3 board.
 * @{
 *
 * @file
 * @brief       Board specific definitions for the iotlab-m3 board.
 *
 * @author      Alaeddine Weslati <alaeddine.weslati@inria.fr>
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <stdint.h>

#include "cpu.h"
#include "periph_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Define the nominal CPU core clock in this board
 */
#define F_CPU               CLOCK_CORECLOCK

/**
 * @name Set the default baudrate to 500K for this board
 * @{
 */
#ifndef STDIO_BAUDRATE
#   define STDIO_BAUDRATE   (500000U)
#endif
/** @} */

/**
 * @name Define the interface to the AT86RF231 radio
 *
 * {spi bus, spi speed, cs pin, int pin, reset pin, sleep pin}
 */
#define AT86RF2XX_PARAMS_BOARD      {.spi = SPI_0, \
                                     .spi_speed = SPI_SPEED_5MHZ, \
                                     .cs_pin = GPIO_PIN(PORT_A, 4), \
                                     .int_pin = GPIO_PIN(PORT_C, 4), \
                                     .sleep_pin = GPIO_PIN(PORT_A, 2), \
                                     .reset_pin = GPIO_PIN(PORT_C, 1)}

/**
 * @name Define the interface to the ISL29020 light sensor
 * @{
 */
#define ISL29020_PARAMS_BOARD       {.i2c = I2C_DEV(0), \
                                     .addr = (0x44), \
                                     .range = ISL29020_RANGE_16K, \
                                     .mode = ISL29020_MODE_AMBIENT}
/** @} */

/**
 * @name Define the interface to the LPS331AP pressure sensor
 * @{
 */
#define LPS331AP_PARAMS_BOARD       {.i2c = I2C_DEV(0), \
                                     .addr = (0x5c), \
                                     .rate = LPS331AP_RATE_7HZ}
/** @} */

/**
 * @name Define the interface for the L3G4200D gyroscope
 * @{
 */
#define L3G4200D_PARAMS_BOARD       {.i2c = I2C_DEV(0), \
                                     .addr = (0x68), \
                                     .int1_pin = GPIO_PIN(PORT_C, 5), \
                                     .int2_pin = GPIO_PIN(PORT_C, 0), \
                                     .mode = L3G4200D_MODE_200_25, \
                                     .scale = L3G4200D_SCALE_500DPS}
/** @} */

/**
 * @name Define the interface to the LSM303DLHC accelerometer and magnetometer
 */
#define LSM303DLHC_PARAMS_BOARD     {.i2c = I2C_DEV(0), \
                                     .acc_addr = LSM303DLHC_ACC_DEFAULT_ADDRESS, \
                                     .acc_pin = GPIO_PIN(PORT_B, 12), \
                                     .acc_rate = LSM303DLHC_ACC_SAMPLE_RATE_10HZ, \
                                     .acc_scale = LSM303DLHC_ACC_SCALE_4G, \
                                     .mag_addr = LSM303DLHC_MAG_DEFAULT_ADDRESS, \
                                     .mag_pin = GPIO_PIN(PORT_B, 2), \
                                     .mag_rate = LSM303DLHC_MAG_SAMPLE_RATE_15HZ, \
                                     .mag_gain = LSM303DLHC_MAG_GAIN_450_400_GAUSS}

/**
 * @name Define the interface for the connected flash memory
 * @{
 */
#define EXTFLASH_SPI        SPI_1
#define EXTFLASH_CS         GPIO_PIN(PORT_A,11)
#define EXTFLASH_WRITE      GPIO_PIN(PORT_C,6)
#define EXTFLASH_HOLD       GPIO_PIN(PORT_C,9)
/** @} */

/**
 * @name LED pin definitions
 * @{
 */
#define LED_RED_PORT        (GPIOD)
#define LED_RED_PIN         (2)
#define LED_RED_GPIO        GPIO_PIN(PORT_D,2)
#define LED_GREEN_PORT      (GPIOB)
#define LED_GREEN_PIN       (5)
#define LED_GREEN_GPIO      GPIO_PIN(PORT_B,5)
#define LED_ORANGE_PORT     (GPIOC)
#define LED_ORANGE_PIN      (10)
#define LED_ORANGE_GPIO     GPIO_PIN(PORT_C,10)
/** @} */

/**
 * @name Macros for controlling the on-board LEDs.
 * @{
 */
#define LED_RED_ON          (LED_RED_PORT->ODR &= ~(1<<LED_RED_PIN))
#define LED_RED_OFF         (LED_RED_PORT->ODR |= (1<<LED_RED_PIN))
#define LED_RED_TOGGLE      (LED_RED_PORT->ODR ^= (1<<LED_RED_PIN))

#define LED_GREEN_ON        (LED_GREEN_PORT->ODR &= ~(1<<LED_GREEN_PIN))
#define LED_GREEN_OFF       (LED_GREEN_PORT->ODR |= (1<<LED_GREEN_PIN))
#define LED_GREEN_TOGGLE    (LED_GREEN_PORT->ODR ^= (1<<LED_GREEN_PIN))

#define LED_ORANGE_ON       (LED_ORANGE_PORT->ODR &= ~(1<<LED_ORANGE_PIN))
#define LED_ORANGE_OFF      (LED_ORANGE_PORT->ODR |= (1<<LED_ORANGE_PIN))
#define LED_ORANGE_TOGGLE   (LED_ORANGE_PORT->ODR ^= (1<<LED_ORANGE_PIN))
/** @} */

/**
 * @brief SAUL GPIO mapping
 */
#define SAUL_GPIO_BOARD       {.pin = LED_RED_PIN, \
                               .dir = GPIO_DIR_OUT, \
                               .name = "LED(red)"}, \
                              {.pin = LED_GREEN_PIN, \
                               .dir = GPIO_DIR_OUT, \
                               .name = "LED(green)"},\
                              {.pin = LED_ORANGE_PIN, \
                               .dir = GPIO_DIR_OUT, \
                               .name = "LED(orange)"}

/**
 * @name xtimer tuning values
 * @{
 */
#define XTIMER_OVERHEAD     6
#define XTIMER_SHOOT_EARLY  3
/** @} */

/**
 * @brief Initialize board specific hardware, including clock, LEDs and std-IO
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H_ */
/** @} */
