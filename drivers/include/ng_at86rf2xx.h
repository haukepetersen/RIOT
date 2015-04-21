/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    drivers_ng_at86rf2xx AT86RF2xx based drivers
 * @ingroup     drivers
 *
 * This module contains drivers for radio devices in Atmel's AT86RF2xx series.
 * The driver is aimed to work with all devices of this series. It does however
 * in its current version only cover some basic functionality.
 *
 * @{
 *
 * @file
 * @brief       Interface definition for AT86RF2xx based drivers
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NG_AT86RF2XX_H_
#define NG_AT86RF2XX_H_

#include <stdint.h>

#include "board.h"
#include "periph/spi.h"
#include "periph/gpio.h"

#include "net/ng_netdev.h"

#include "ng_at86rf2xx.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief at86rf231's minimum TX power in dBm
 */
#define NG_AT86RF2XX_TX_POWER_MIN       -17

/**
 * @brief at86rf231's maximum TX power in dBm
 */
#define NG_AT86RF2XX_TX_POWER_MAX       4

/**
  * @brief at86rf2xx channel config
  * @{
  */
#ifndef MODULE_AT86RF212B
#define NG_AT86RF2XX_MIN_CHANNEL        (11U)
#define NG_AT86RF2XX_MAX_CHANNEL        (26U)
#define NG_AT86RF2XX_DEFAULT_CHANNEL    (17U)
#else
/* 212b has a sub-1GHz radio */
#define NG_AT86RF2XX_MIN_CHANNEL        (0)
#define NG_AT86RF2XX_MAX_CHANNEL        (10)
#define NG_AT86RF2XX_DEFAULT_CHANNEL    (5)
#endif
/** @} */

/**
  * @brief at86rf231's highest supported channel
  */

#define NG_AT86RF2XX_MAX_PKT_LENGTH     (127)

#define NG_AT86RF2XX_DEFAULT_ADDR_SHORT (0x0230)
#define NG_AT86RF2XX_DEFAULT_ADDR_LONG  (0x1122334455667788)

#define NG_AT86RF2XX_DEFAULT_PANID      (0xffff)
#define NG_AT86RF2XX_DEFAULT_TXPOWER    (0U)

typedef enum {
    STATE_TRX_OFF       = 0x08,
    STATE_PLL_ON        = 0x09,
    STATE_SLEEP         = 0x0f,
    STATE_BUSY_RX_AACK  = 0x11,
    STATE_BUSY_TX_ARET  = 0x12,
    STATE_RX_AACK_ON    = 0x16,
    STATE_TX_ARET_ON    = 0x19,
    STATE_IN_PROGRESS   = 0x1f,
} ng_at86rf2xx_state_t;

typedef enum {
    OPT_AUTOACK         = 0x0001,
    OPT_CSMA            = 0x0002,
    OPT_PROMISCUOUS     = 0x0004,
    OPT_PRELOADING      = 0x0008,
    OPT_TELL_TX_START   = 0x0010,
    OPT_TELL_TX_END     = 0x0020,
    OPT_TELL_RX_START   = 0x0040,
    OPT_TELL_RX_END     = 0x0080,
    OPT_RAWDUMP         = 0x0100,
} ng_at86rf2xx_opt_t;

/**
 * @brief Device descriptor for AT86RF2XX radio devices
 * @details First three fields are for compliance to ng_netdev_t
 */
typedef struct {
    /* netdev fields */
    const ng_netdev_driver_t *driver;     /**< pointer to the devices interface */
    ng_netdev_event_cb_t event_cb;  /**< netdev event callback */
    kernel_pid_t mac_pid;           /**< the driver's thread's PID */
    /* device specific fields */
    spi_t spi;                      /**< SPI device the radio device is connected to */
    gpio_t cs_pin;                  /**< Radio device's chip select pin */
    gpio_t sleep_pin;               /**< Radio device's sleep pin */
    gpio_t reset_pin;               /**< Radio device's reset pin */
    gpio_t int_pin;                 /**< Radio device's interrupt pin */
    ng_nettype_t proto;             /**< Protocol the radio expects */
    ng_netconf_state_t state;       /**< Variable to keep radio driver's state */
    uint8_t seq_nr;                 /**< Next packets sequence number */
    uint8_t frame_len;              /**< Length of the current TX frame */
    uint16_t pan;                   /**< The PAN the radio device is using */
    uint16_t addr_short;            /**< The short address the radio device is using */
    uint64_t addr_long;             /**< The long address the radio device is using */
    uint16_t options;               /**< Bit field to save enable/disable options */
    ng_at86rf2xx_state_t idle_state;/**< State to return to after sending */
} ng_at86rf2xx_t;


/* TODO: lots of doxygen missing here... */

int ng_at86rf2xx_init(ng_at86rf2xx_t *dev, spi_t spi, spi_speed_t spi_speed,
                      gpio_t cs_pin, gpio_t int_pin,
                      gpio_t sleep_pin, gpio_t reset_pin);

void ng_at86rf2xx_reset(ng_at86rf2xx_t *dev);

bool ng_at86rf2xx_cca(ng_at86rf2xx_t *dev);

uint16_t ng_at86rf2xx_get_addr_short(ng_at86rf2xx_t *dev);
void ng_at86rf2xx_set_addr_short(ng_at86rf2xx_t *dev, uint16_t addr);

uint64_t ng_at86rf2xx_get_addr_long(ng_at86rf2xx_t *dev);
void ng_at86rf2xx_set_addr_long(ng_at86rf2xx_t *dev, uint64_t addr);

uint8_t ng_at86rf2xx_get_chan(ng_at86rf2xx_t *dev);
void ng_at86rf2xx_set_chan(ng_at86rf2xx_t *dev, uint8_t chan);

uint16_t ng_at86rf2xx_get_pan(ng_at86rf2xx_t *dev);
void ng_at86rf2xx_set_pan(ng_at86rf2xx_t *dev, uint16_t pan);

int16_t ng_at86rf2xx_get_txpower(ng_at86rf2xx_t *dev);
void ng_at86rf2xx_set_txpower(ng_at86rf2xx_t *dev, int16_t txpower);

void ng_at86rf2xx_set_option(ng_at86rf2xx_t *dev,
                             ng_at86rf2xx_opt_t option, bool state);

ng_at86rf2xx_state_t ng_at86rf2xx_get_state(ng_at86rf2xx_t *dev);
void ng_at86rf2xx_set_state(ng_at86rf2xx_t *dev, ng_at86rf2xx_state_t state);

size_t ng_at86rf2xx_send(ng_at86rf2xx_t *dev, uint8_t *data, size_t len);

void ng_at86rf2xx_tx_prepare(ng_at86rf2xx_t *dev);
size_t ng_at86rf2xx_tx_load(ng_at86rf2xx_t *dev, uint8_t *data, size_t len,
                            size_t offset);
void ng_at86rf2xx_tx_exec(ng_at86rf2xx_t *dev);

size_t ng_at86rf2xx_rx_len(ng_at86rf2xx_t *dev);
void ng_at86rf2xx_rx_read(ng_at86rf2xx_t *dev, uint8_t *data, size_t len,
                          size_t offset);

#ifdef __cplusplus
}
#endif

#endif /* NG_AT86RF2XX_H_ */
/** @} */
