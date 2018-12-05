/*
 * Copyright (C) 2017 Frits Kuipers
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ds18
 *
 * @{
 * @file
 * @brief       Default configuration for DS1822 and DS18B20 temperature sensors
 *
 * @author      Frits Kuipers <frits.kuipers@gmail.com>
 */

#ifndef DS18_PARAMS_H
#define DS18_PARAMS_H

#include "ds18.h"
#include "saul_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Set default configuration parameters for the ds18
 *          Note that the data pin is used alternately for input and output.
 *          The input setting is deducted from the output setting: if the
 *          setting here is to use internal pull up resistors during output, then
 *          they will be used for input, too (even if output resistors are not
 *          actually supported by the hardware). See ds18_init code for further
 *          details.
 * @{
 */
#ifndef DS18_PARAM_PIN
#define DS18_PARAM_PIN             (GPIO_PIN(PB, 9))
#endif
#ifndef DS18_PARAM_PULL
#define DS18_PARAM_PULL            (GPIO_OD_PU)
#endif

#define DS18_PARAMS_DEFAULT        { .pin     = DS18_PARAM_PIN, \
                                     .out_mode = DS18_PARAM_PULL }
/**@}*/

/**
 * @brief   Configure ds18
 */
static const ds18_params_t ds18_params[] =
{
#ifdef DS18_PARAMS_BOARD
    DS18_PARAMS_BOARD,
#else
    DS18_PARAMS_DEFAULT,
#endif
};

/**
 * @brief   Configure SAUL registry entries
 */
static const saul_reg_info_t ds18_saul_reg_info[] =
{
    { .name = "ds18" }
};

#ifdef __cplusplus
}
#endif

#endif /* DS18_PARAMS_H */
/** @} */
