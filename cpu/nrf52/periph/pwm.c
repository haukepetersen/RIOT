/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_nrf52
 * @{
 *
 * @file
 * @brief       Implementation of the peripheral PWM interface
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "periph/pwm.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"


/**
 * @brief   Define masks to get the MODE register value and the polarity from
 *          the given pwm_mode_t value
 * @{
 */
#define POL_MASK        (0x8000)
#define MODE_MASK       (0x0001)
/** @} */

/**
 * @brief   When calculating the actual frequency, we allow for a deviation of
 *          ~15.5% (val +- (val / 8))
 */
#define F_DEV           (8)

/**
 * @brief   Allocate some memory for the PWM sequences
 */
static uint16_t pwm_seq[PWM_NUMOF][PWM_CHANNELS];

static inline NRF_PWM_Type *dev(pwm_t pwm)
{
    return pwm_config[pwm].dev;
}

uint32_t pwm_init(pwm_t pwm, pwm_mode_t mode, uint32_t freq, uint16_t res)
{
    uint32_t real_clk;

    /* check if given device is valid */
    if ((pwm >= PWM_NUMOF) || (res > 0x7fff)) {
        return 0;
    }

    /* make sure the device is stopped */
    dev(pwm)->TASKS_STOP = 1;
    dev(pwm)->ENABLE = 0;

    /* calculate the needed frequency, for center modes we need double */
    uint32_t clk = ((freq * res) << (mode & 0x1));
    /* match to best fitting prescaler */
    for (int i = 0; i < 8; i++) {
        real_clk = (PERIPH_CLOCK >> i);
        if (((real_clk - (real_clk / F_DEV)) < clk) &&
            ((real_clk + (real_clk / F_DEV)) > clk)) {
            break;
        }
        if (i == 7) {
            return 0;
        }
    }
    real_clk /= res;

    /* pin configuration */
    for (unsigned i = 0; i < PWM_CHANNELS; i++) {
        if (pwm_config[pwm].pin[i] != GPIO_UNDEF) {
            NRF_P0->PIN_CNF[pwm_config[pwm].pin[i]] = 0x3;
        }
        pwm_seq[pwm][i] = (POL_MASK & mode);
        dev(pwm)->PSEL.OUT[i] = pwm_config[pwm].pin[i];
        DEBUG("set PIN[%i] to %i\n", (int)i, (int)pwm_config[pwm].pin[i]);
    }

    /* enable the device */
    dev(pwm)->ENABLE = 1;

    /* finally get the actual selected frequency */

    DEBUG("set real f to %i\n", (int)real_clk);
    dev(pwm)->COUNTERTOP = (res - 1);

    /* select PWM mode */
    dev(pwm)->MODE = (mode & 0x1);
    dev(pwm)->LOOP = 0;
    dev(pwm)->DECODER = PWM_DECODER_LOAD_Individual;

    DEBUG("MODE: 0x%08x\n", (int)dev(pwm)->MODE);
    DEBUG("LOOP: 0x%08x\n", (int)dev(pwm)->LOOP);
    DEBUG("DECODER: 0x%08x\n", (int)dev(pwm)->DECODER);

    /* setup the sequence */
    dev(pwm)->SEQ[0].PTR = (uint32_t)pwm_seq[pwm];
    dev(pwm)->SEQ[0].CNT = pwm_channels(pwm);
    dev(pwm)->SEQ[0].REFRESH = 0;
    dev(pwm)->SEQ[0].ENDDELAY = 0;

    DEBUG("ptr: 0x%08x\n", (int)dev(pwm)->SEQ[0].PTR);
    DEBUG("cnt: 0x%08x\n", (int)dev(pwm)->SEQ[0].CNT);
    DEBUG("refresh: 0x%08x\n", (int)dev(pwm)->SEQ[0].REFRESH);
    DEBUG("enddelay: 0x%08x\n", (int)dev(pwm)->SEQ[0].ENDDELAY);

    dev(pwm)->INTENSET = 0xffffffff;

    DEBUG("inten: 0x%08x\n", (int)dev(pwm)->INTEN);

    /* start sequence */
    dev(pwm)->TASKS_SEQSTART[0] = 1;

    DEBUG("started\n");

    return real_clk;
}

uint8_t pwm_channels(pwm_t pwm)
{
    uint8_t channels = 0;

    while ((channels < PWM_CHANNELS) &&
           (pwm_config[pwm].pin[channels] != GPIO_UNDEF)) {
        ++channels;
    }

    return channels;
}

void pwm_set(pwm_t pwm, uint8_t channel, uint16_t value)
{
    if ((channel >= PWM_CHANNELS) || (value > 0x7fff)) {
        return;
    }

    DEBUG("Setting ch %i to %i\n", (int)channel, (int)value);

    value |= (pwm_seq[pwm][channel] & POL_MASK);
    DEBUG("value: 0x%04x\n", (int)value);
    pwm_seq[pwm][channel] = value;
    DEBUG("v set: 0x%04x\n", (int)pwm_seq[pwm][channel]);
}

void pwm_stop(pwm_t pwm)
{
    DEBUG("STOPPING PWM %i\n", (int)pwm);
    dev(pwm)->TASKS_STOP = 1;
    dev(pwm)->ENABLE = 0;
}

void pwm_start(pwm_t pwm)
{
    DEBUG("STARTING PWM %i\n", (int)pwm);
    dev(pwm)->ENABLE = 1;
    dev(pwm)->TASKS_SEQSTART[0] = 1;
}

void pwm_poweron(pwm_t pwm)
{
    /* nothing to do here */
}

void pwm_poweroff(pwm_t pwm)
{
    /* nothing to do here */
}

void isr_pwm0(void)
{
    puts("ISR");

    if (NRF_PWM0->EVENTS_STOPPED == 1) {
        puts("isr evt stopped");
    }
    for (int i = 0; i < 2; i++) {
        if (NRF_PWM0->EVENTS_SEQSTARTED[i]) {
            printf("isr seq %i started\n", i);
        }
        if (NRF_PWM0->EVENTS_SEQEND[i]) {
            printf("isr seq %i end\n", i);
        }
    }
    if (NRF_PWM0->EVENTS_PWMPERIODEND == 1) {
        puts("isr pwmperiodend");
    }
    if (NRF_PWM0->EVENTS_LOOPSDONE == 1) {
        puts("isr loopsdone");
    }
}
