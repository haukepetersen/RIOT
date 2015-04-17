/*
 * Copyright (C) 2013 Alaeddine Weslati <alaeddine.weslati@inria.fr>
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     drivers_ng_at86rf2xx
 * @{
 *
 * @file
 * @brief       Internal interfaces for AT86RF2xx drivers
 *
 * @author      Alaeddine Weslati <alaeddine.weslati@inria.fr>
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */


#ifndef NG_AT86RF2XX_INTERNAL_H_
#define NG_AT86RF2XX_INTERNAL_H_

#include <stdint.h>

#include "ng_at86rf2xx.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read from a register at address `addr` from device `dev`.
 *
 * @param[in] dev   Device descriptor of the radio device to read from
 * @param[in] addr  Address of the radio device's register
 *
 * @return the value of the specified register
 */
uint8_t ng_at86rf2xx_reg_read(const ng_at86rf2xx_t *dev, const uint8_t addr);

/**
 * @brief Write to a register at address `addr` from device `dev`.
 *
 * @param[in] dev       Device descriptor of the radio device to write to
 * @param[in] addr      Address of the radio device's register
 * @param[in] value     The value to write to the device's register
 */
void ng_at86rf2xx_reg_write(const ng_at86rf2xx_t *dev, const uint8_t addr,
                            const uint8_t value);

/**
 * @brief Read `length` number of bytes into `data` from device `dev`'s packet FIFO
 *        starting from `addr`.
 * @details This function provides access to dedicated bytes within the FIFO.
 *
 * @param[in]   dev     device descriptor of the radio device to read from
 * @param[in]   offset  offset to start reading from. Valid values [0x00 - 0x7f]
 * @param[out]  data    buffer to copy FIFO data into
 * @param[in]   length  number of bytes to read from FIFO
 */
void ng_at86rf2xx_sram_read(const ng_at86rf2xx_t *dev,
                            const uint8_t offset,
                            uint8_t *data,
                            const size_t length);

/**
 * @brief Write `length` number of bytes from `data` to device `dev`'s packet FIFO
 *        starting from `addr` in the FIFO.
 * @details This function provides access to dedicated bytes within the FIFO.
 *
 * @param[in]   dev     device descriptor of the radio device to write to
 * @param[in]   offset  offset to start writing to. Valid values [0x00 - 0x7f]
 * @param[in]   data    data to copy into FIFO
 * @param[in]   length  number of bytes to write to FIFO
 */
void ng_at86rf2xx_sram_write(const ng_at86rf2xx_t *dev,
                             const uint8_t offset,
                             const uint8_t *data,
                             const size_t length);

/**
 * @brief [brief description]
 *
 * @param dev [description]
 * @param data [description]
 * @param len [description]
 */
void ng_at86rf2xx_fb_read(const ng_at86rf2xx_t *dev,
                          uint8_t *data, const size_t len);

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param dev [description]
 * @param data [description]
 * @param len [description]
 */
void ng_at86rf2xx_fb_write(const ng_at86rf2xx_t *dev,
                           const uint8_t *data,
                           const size_t len);

/**
 * @brief [brief description]
 *
 * @return [description]
 */
uint8_t ng_at86rf2xx_get_status(const ng_at86rf2xx_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* NG_AT86RF2XX_INTERNAL_H_ */
/** @} */
