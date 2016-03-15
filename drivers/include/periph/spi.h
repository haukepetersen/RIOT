/*
 * Copyright (C) 2014-2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_periph_spi SPI
 * @ingroup     drivers_periph
 * @brief       Low-level SPI peripheral driver
 *
 * @{
 * @file
 * @brief       Low-level SPI peripheral driver interface definitions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef PERIPH_SPI_H
#define PERIPH_SPI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include "periph_cpu.h"
#include "periph_conf.h"
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Default SPI device access macro
 */
#ifndef SPI_DEV
#define SPI_DEV(x)      (x)
#endif

/**
 * @brief   Define global value for undefined SPI device
 */
#ifndef SPI_UNDEF
#define SPI_UNDEF       (UINT_MAX)
#endif

/**
 * @brief   Default SPI chip select pin access macro
 *
 * UINT_MAX is the default value for GPIO_UNDEF, so we go one less here. This
 * still gives us the highest possible chance not to collide with and valid
 * GPIO_PIN(x,y) value.
 */
#ifndef SPI_HWCS
#define SPI_HWCS(x)     (UINT_MAX - 1 - x)
#endif

/**
 * @brief   Define value for unused CS line
 */
#ifndef SPI_CS_UNDEF
#define SPI_CS_UNDEF    (GPIO_UNDEF)
#endif

/**
 * @brief   Default type for SPI devices
 */
#ifndef HAVE_SPI_T
typedef unsigned int spi_t;
#endif

/**
 * @brief   Chip select pin type overlaps with gpio_t so it can be casted to
 *          this
 */
#ifndef HAVE_SPI_CS_T
typedef gpio_t spi_cs_t;
#endif

/**
 * @brief   Available SPI modes, defining the configuration of clock polarity
 *          and clock phase
 *
 * RIOT is using the mode numbers as commonly defined by most vendors
 * (https://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus#Mode_numbers):
 *
 * - MODE_0: CPOL=0, CPHA=0 - The first data bit is sampled by the receiver on
 *           the first SCK rising SCK edge (this mode is used most often).
 * - MODE_1: CPOL=0, CPHA=1 - The first data bit is sampled by the receiver on
 *           the second rising SCK edge.
 * - MODE_2: CPOL=1, CPHA=0 - The first data bit is sampled by the receiver on
 *           the first falling SCK edge.
 * - MODE_3: CPOL=1, CPHA=1 - The first data bit is sampled by the receiver on
 *           the second falling SCK edge.
 */
#ifndef HAVE_SPI_MODE_T
typedef enum {
    SPI_MODE_0,             /**< CPOL=0, CPHA=0 */
    SPI_MODE_1,             /**< CPOL=0, CPHA=1 */
    SPI_MODE_2,             /**< CPOL=1, CPHA=0 */
    SPI_MODE_3              /**< CPOL=1, CPHA=1 */
} spi_mode_t;
#endif

/**
 * @brief   Available SPI clock speeds
 *
 * The actual speed of the bus can vary to some extend, as the combination of
 * CPU clock and available prescaler values on certain platforms may not make
 * the exact values possible.
 */
#ifndef HAVE_SPI_CLK_T
typedef enum {
    SPI_CLK_100KHZ,         /**< drive the SPI bus with 100KHz */
    SPI_CLK_400KHZ,         /**< drive the SPI bus with 400KHz */
    SPI_CLK_1MHZ,           /**< drive the SPI bus with 1MHz */
    SPI_CLK_5MHZ,           /**< drive the SPI bus with 5MHz */
    SPI_CLK_10MHZ           /**< drive the SPI bus with 10MHz */
} spi_clk_t;
#endif

/**
 * @brief   Initialize pins used by the given SPI device
 *
 * @param[in] dev       initialize pins for this SPI device
 * @param[in] cs        also initialize this chip select pin
 *
 * @return              0 on success
 * @return              -1 on invalid device
 * @return              -2 on invalid CS
 */
int spi_init(spi_t dev, spi_cs_t cs);

/**
 * @brief   Start a new SPI transaction
 *
 * Starting a new SPI transaction will get exclusive access to the SPI bus
 * and configure it according to the given values. If another SPI transaction
 * is active when this function is called, this function will block until the
 * other transaction is complete (spi_relase was called).
 *
 * @param[in] dev       SPI device to access
 * @param[in] mode      mode to use for the new transaction
 * @param[in] clk       bus clock speed to use for the transaction
 * @param[in] cs        chip select pin to use
 *
 * @return              0 on success
 * @return              -1 on error
 */
int spi_acquire(spi_t dev, spi_mode_t mode, spi_clk_t clk, spi_cs_t cs);

/**
 * @brief Release the given SPI device to be used by others
 *
 * @param[in] dev       SPI device to release
 */
void spi_release(spi_t dev);

/**
 * @brief Transfer one byte on the given SPI bus
 *
 * @param[in] dev       SPI device to use
 * @param[in] out       Byte to send out, set NULL if only receiving
 *
 * @return              The byte that was received
 */
uint8_t spi_transfer_byte(spi_t bus, spi_cs_t cs, bool cont, uint8_t out);

/**
 * @brief Transfer a number bytes on the given SPI bus
 *
 * @param[in] dev       SPI device to use
 * @param[in] out       Array of bytes to send, set NULL if only receiving
 * @param[out] in       Buffer to receive bytes to, set NULL if only sending
 * @param[in] length    Number of bytes to transfer
 */
void spi_transfer_bytes(spi_t bus, spi_cs_t cs, bool cont,
                        uint8_t *out, uint8_t *in, size_t len);

/**
 * @brief Transfer one byte to/from a given register address
 *
 * This function is a shortcut function for easier handling of register based SPI devices. As
 * many SPI devices use a register based addressing scheme, this function is a convenient short-
 * cut for interfacing with such devices.
 *
 * @param[in] dev       SPI device to use
 * @param[in] reg       Register address to transfer data to/from
 * @param[in] out       Byte to send, set NULL if only receiving data
 * @param[out] in       Byte to read, set NULL if only sending
 *
 * @return              The value that was read from the given register address
 */
uint8_t spi_transfer_reg(spi_t bus, spi_cs_t cs, uint8_t reg, uint8_t out);

/**
 * @brief Transfer a number of bytes from/to a given register address
 *
 * This function is a shortcut function for easier handling of register based SPI devices. As
 * many SPI devices use a register based addressing scheme, this function is a convenient short-
 * cut for interfacing with such devices.
 *
 * @param[in] dev       SPI device to use
 * @param[in] reg       Register address to transfer data to/from
 * @param[in] out       Byte array to send data from, set NULL if only receiving
 * @param[out] in       Byte buffer to read into, set NULL if only sending
 * @param[in] length    Number of bytes to transfer
 */
void spi_transfer_regs(spi_t bus, spi_cs_t cs,
                       uint8_t reg, uint8_t *out, uint8_t *in, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_SPI_H */
/** @} */
