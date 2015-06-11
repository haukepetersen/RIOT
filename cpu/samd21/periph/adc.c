/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_samr21
 * @{
 *
 * @file
 * @brief       Implementation of ADC peripheral driver
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "cpu.h"
#include "periph/adc.h"

static int _get_max(void)
{
    switch (ADC->CTRLB.reg & ADC_CTRLB_RESSEL_Msk) {
        case ADC_CTRLB_RESSEL_12BIT:
            return 0xfff;
        case ADC_CTRLB_RESSEL_10BIT:
            return 0x3ff;
        case ADC_CTRLB_RESSEL_8BIT_Val:
            return 0x0ff;
        default:
            return 0;
    }
}

int adc_init(adc_t dev, adc_precision_t precision)
{
    uint32_t reso;

    if (dev != ADC_0) {
        return -2;
    }

    /* config ure used pins */
    for (int i = 0; i < ADC_CHANNELS; i++) {
        PortGroup *port = adc_config[i].port;
        int pin = adc_config[i].pin;
        port->DIRCLR.reg = (1 << pin);
        port->PINCFG[pin].reg = (PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN);
        port->PMUX[pin >> 1].reg = ~(0xf << (4 * (pin & 0x1)));
        port->PMUX[pin >> 1].reg =  (1 << (4 * (pin & 0x1)));
    }

    /* get resolution */
    switch (precision) {
        case ADC_RES_8BIT:
            reso = ADC_CTRLB_RESSEL_8BIT;
            break;
        case ADC_RES_10BIT:
            reso = ADC_CTRLB_RESSEL_10BIT;
            break;
        case ADC_RES_12BIT:
            reso = ADC_CTRLB_RESSEL_12BIT;
            break;
        case ADC_RES_6BIT:
        case ADC_RES_14BIT:
        case ADC_RES_16BIT:
        default:
            return -1;
    }

    /* power on the ADC */
    adc_poweron(dev);
    /* reset ADC module */
    ADC->CTRLA.reg = ADC_CTRLA_SWRST;
    while (ADC->CTRLA.reg & ADC_CTRLA_SWRST ||
           ADC->STATUS.reg & ADC_STATUS_SYNCBUSY);
    /* select the reference voltage to be VDDANA / 2 -> 1.66V */
    ADC->REFCTRL.reg = ADC_REFCTRL_REFSEL_INTVCC1;
    /* set prescaler to 16 and set resolution */
    ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV16 | reso;
    /* set sampling time to 1 as save value */
    ADC->SAMPCTRL.reg = ADC_SAMPCTRL_SAMPLEN(1);

    /* enable the ADC */
    ADC->CTRLA.reg = ADC_CTRLA_ENABLE;
    return 0;
}

int adc_sample(adc_t dev, int channel)
{
    if (dev != ADC_0) {
        return -2;
    }
    if (channel >= ADC_CHANNELS) {
        return -1;
    }

    /* select the input pin (and GND as negative mux) */
    ADC->INPUTCTRL.reg = adc_config[channel].muxpos | ADC_INPUTCTRL_MUXNEG_IOGND;
    while (ADC->STATUS.reg & ADC_STATUS_SYNCBUSY);
    /* start the conversion */
    ADC->SWTRIG.reg = ADC_SWTRIG_START;
    /* wait for the result */
    while (!(ADC->INTFLAG.reg & ADC_INTFLAG_RESRDY));
    /* read result */
    return (int)ADC->RESULT.reg;
}

void adc_poweron(adc_t dev)
{
    if (dev != ADC_0) {
        return;
    }
    GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_CLKEN |
                         (ADC_GCLK_ID << GCLK_CLKCTRL_ID_Pos));
    PM->APBCMASK.reg |= PM_APBCMASK_ADC;

}

void adc_poweroff(adc_t dev)
{
    if (dev != ADC_0) {
        return;
    }
    GCLK->CLKCTRL.reg = (ADC_GCLK_ID << GCLK_CLKCTRL_ID_Pos);
    PM->APBCMASK.reg &= ~(PM_APBCMASK_ADC);

}

int adc_map(adc_t dev, int value, int min, int max)
{
    return (int)adc_mapf(dev, value, (float)min, (float)max);
}

float adc_mapf(adc_t dev, int value, float min, float max)
{
    return ((max - min) / ((float)_get_max())) * value;
}
