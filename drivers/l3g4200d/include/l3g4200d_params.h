/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup   drivers_l3g4200d
 * @{
 *
 * @file
 * @brief     Default configuration for L3G4220D devices
 *
 * @author    Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef L3G4200D_PARAMS_H
#define L3G4200D_PARAMS_H

#include "board.h"
#include "l3g4200d.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Default parameters for L3G4200D devices
 * @{
 */
#ifndef L3G4200D_PARAM_I2C
#define L3G4200D_PARAM_I2C          (I2C_DEV(0))
#endif
#ifndef L3G4200D_PARAM_ADDR
#define L3G4200D_PARAM_ADDR         (L3G4200D_DEFAULT_ADDRESS)
#endif
#ifndef L3G4200D_PARAM_INT1_PIN
#define L3G4200D_PARAM_INT1_PIN     (GPIO_UNDEF)
#endif
#ifndef L3G4200D_PARAM_INT2_PIN
#define L3G4200D_PARAM_INT2_PIN     (GPIO_UNDEF)
#endif
#ifndef L3G4200D_PARAM_MODE
#define L3G4200D_PARAM_MODE         (L3G4200D_MODE_200_25)
#endif
#ifndef L3G4200D_PARAM_SCALE
#define L3G4200D_PARAM_SCALE        (L3G4200D_SCALE_500DPS)
#endif

#define L3G4200D_PARAMS_DEFAULT     {.i2c = L3G4200D_PARAM_I2C, \
                                     .addr = L3G4200D_PARAM_ADDR, \
                                     .int1_pin = L3G4200D_PARAM_INT1_PIN, \
                                     .int2_pin = L3G4200D_PARAM_INT2_PIN, \
                                     .mode = L3G4200D_PARAM_MODE, \
                                     .scale = L3G4200D_PARAM_SCALE}
/** @} */

/**
 * @brief   L3G4200D configuration
 */
static const  l3g4200d_params_t l3g4200d_params[] =
{
#ifdef L3G4200D_PARAMS_BOARD
    L3G4200D_PARAMS_BOARD
#else
    L3G4200D_PARAMS_DEFAULT
#endif
};

/**
 * @brief   Get the number of defined L3G4200D devices
 */
#define L3G4200D_NUMOF  (sizeof(l3g4200d_params) / sizeof(l3g4200d_params[0]))

/**
 * @brief   Additional meta information to keep in the SAUL registry
 */
saul_reg_t l3g4200d_saul[] =
{
    {
        .name = "l3g4200d",
        .driver = &l3g4200d_saul_driver
    }
};

#ifdef __cplusplus
}
#endif

#endif /* L3G4200D_PARAMS_H */
/** @} */
