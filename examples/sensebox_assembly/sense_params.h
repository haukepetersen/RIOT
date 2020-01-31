/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     app
 * @brief       Sensor configuration
 * @{
 *
 * @file
 * @brief       App specific sensor configuration
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef SENSE_PARAMS_H
#define SENSE_PARAMS_H

#include "hdc1000.h"
#include "tsl4531x.h"
#include "ds18.h"
#include "periph/adc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HYGRO_NUMOF             (4U)
#define HYGRO_RES               ADC_RES_10BIT

#define SENSE_NUMOF             (HYGRO_NUMOF + 4U)

#define SENSE_PWR_PIN           GPIO_PIN(PB, 8)
#define SENSE_PWR_STARTDELAY    (5U)
#define SENSE_PWR_TESTCYCLES    (3U)
#define SENSE_PWR_TESTDELAY     (1U)

#ifdef __cplusplus
}
#endif

#endif /* SENSE_PARAMS_H */
