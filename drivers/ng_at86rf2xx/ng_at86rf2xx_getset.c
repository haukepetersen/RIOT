/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ng_at86rf2xx
 * @{
 *
 * @file
 * @brief       Getter and setter functions for the AT86RF2xx drivers
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "ng_at86rf2xx.h"
#include "ng_at86rf2xx_internal.h"
#include "ng_at86rf2xx_registers.h"
#include "periph/spi.h"

#define ENABLE_DEBUG (0)
#include "debug.h"


static const int16_t tx_pow_to_dbm[] = {3, 3, 2, 2, 1, 1, 0,
                                        -1, -2, -3, -4, -5, -7, -9, -12, -17};

static const uint8_t dbm_to_tx_pow[] = {0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e,
                                        0x0e, 0x0d, 0x0d, 0x0c, 0x0c, 0x0b,
                                        0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06,
                                        0x05, 0x03, 0x00};

uint16_t ng_at86rf2xx_get_addr_short(ng_at86rf2xx_t *dev)
{
    return dev->addr_short;
}

void ng_at86rf2xx_set_addr_short(ng_at86rf2xx_t *dev, uint16_t addr)
{
    dev->addr_short = addr;
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__SHORT_ADDR_0, (addr >> 8));
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__SHORT_ADDR_1, (uint8_t)addr);
}

uint64_t ng_at86rf2xx_get_addr_long(ng_at86rf2xx_t *dev)
{
    return dev->addr_long;
}

void ng_at86rf2xx_set_addr_long(ng_at86rf2xx_t *dev, uint64_t addr)
{
    dev->addr_long = addr;
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__IEEE_ADDR_0, addr);
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__IEEE_ADDR_1, addr >> 8);
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__IEEE_ADDR_2, addr >> 16);
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__IEEE_ADDR_3, addr >> 24);
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__IEEE_ADDR_4, addr >> 32);
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__IEEE_ADDR_5, addr >> 40);
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__IEEE_ADDR_6, addr >> 48);
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__IEEE_ADDR_7, addr >> 56);
}

uint8_t ng_at86rf2xx_get_chan(ng_at86rf2xx_t *dev)
{
    uint8_t res = ng_at86rf2xx_reg_read(dev, NG_AT86RF2XX_REG__PHY_CC_CCA);
    return (res & NG_AT86RF2XX_PHY_CC_CCA_MASK__CHANNEL);
}

void ng_at86rf2xx_set_chan(ng_at86rf2xx_t *dev, uint8_t channel)
{
    uint8_t tmp;

    if (channel < NG_AT86RF2XX_MIN_CHANNEL
        || channel > NG_AT86RF2XX_MAX_CHANNEL) {
        return;
    }
    tmp = ng_at86rf2xx_reg_read(dev, NG_AT86RF2XX_REG__PHY_CC_CCA);
    tmp &= ~(NG_AT86RF2XX_PHY_CC_CCA_MASK__CHANNEL);
    tmp = (channel & NG_AT86RF2XX_PHY_CC_CCA_MASK__CHANNEL);
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__PHY_CC_CCA, tmp);
}

uint16_t ng_at86rf2xx_get_pan(ng_at86rf2xx_t *dev)
{
    return dev->pan;
}

void ng_at86rf2xx_set_pan(ng_at86rf2xx_t *dev, uint16_t pan)
{
    dev->pan = pan;
    DEBUG("pan0: %u, pan1: %u\n", (uint8_t)pan, pan >> 8);
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__PAN_ID_0, (uint8_t)pan);
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__PAN_ID_1, (pan >> 8));
}

int16_t ng_at86rf2xx_get_txpower(ng_at86rf2xx_t *dev)
{
    uint8_t txpower = ng_at86rf2xx_reg_read(dev, NG_AT86RF2XX_REG__PHY_TX_PWR)
                & NG_AT86RF2XX_PHY_TX_PWR_MASK__TX_PWR;
    return tx_pow_to_dbm[txpower];
}

void ng_at86rf2xx_set_txpower(ng_at86rf2xx_t *dev, int16_t txpower)
{
    txpower += 17;
    if (txpower < 0) {
        txpower = 0;
    } else if (txpower > 20) {
        txpower = 20;
    }
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__PHY_TX_PWR,
                           dbm_to_tx_pow[txpower]);
}

void ng_at86rf2xx_set_option(ng_at86rf2xx_t *dev, ng_at86rf2xx_opt_t option,
                             bool state)
{
    /* set option field */
    if (state) {
        dev->options |= option;
    }
    else {
        dev->options &= ~(option);
    }
    /* trigger option specific actions */
    switch (option) {
        case OPT_AUTOACK:
            /* TODO: en/disable auto-acking */
            break;
        case OPT_CSMA:
            /* TODO: en/disable csma */
            break;
        case OPT_PROMISCUOUS:
            /* TODO: en/disable promiscuous mode */
            break;
        default:
            /* do nothing */
            break;
    }
}

ng_at86rf2xx_state_t ng_at86rf2xx_get_state(ng_at86rf2xx_t *dev)
{
    uint8_t status = ng_at86rf2xx_get_status(dev);
    return (status & 0x1f);
}

static inline void _set_state(ng_at86rf2xx_t *dev, ng_at86rf2xx_state_t state)
{
    // int i = 0;
    ng_at86rf2xx_reg_write(dev, NG_AT86RF2XX_REG__TRX_STATE, state);
    while (ng_at86rf2xx_get_state(dev) != state);
    //  {
    //     i ++;
    // }
    // printf("Convergence time: %i cycles\n", i);
}

void ng_at86rf2xx_set_state(ng_at86rf2xx_t *dev, ng_at86rf2xx_state_t state)
{
    uint8_t old_state = ng_at86rf2xx_get_state(dev);

    if (state == old_state) {
        return;
    }
    /* make sure there is no ongoing transmission */
    while (old_state == STATE_BUSY_RX_AACK || old_state == STATE_BUSY_TX_ARET) {
        old_state = ng_at86rf2xx_get_state(dev);
    }
    /* check if we need to wake up from sleep mode */
    if (old_state == STATE_SLEEP) {
        DEBUG("at86rf2xx: waking up from sleep mode\n");
        gpio_clear(dev->sleep_pin);
        while (ng_at86rf2xx_get_state(dev) != STATE_TRX_OFF);
    }
    /* go to neutral TRX_OFF state */
    _set_state(dev, STATE_TRX_OFF);
    if (state == STATE_RX_AACK_ON || state == STATE_TX_ARET_ON) {
        _set_state(dev, state);
    } else if (state == STATE_SLEEP) {
        gpio_set(dev->sleep_pin);
        while (ng_at86rf2xx_get_state(dev) != STATE_SLEEP);
    }
}
