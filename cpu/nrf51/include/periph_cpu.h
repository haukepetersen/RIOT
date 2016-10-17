/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_nrf51
 * @{
 *
 * @file
 * @brief           NRF51 specific definitions for handling peripherals
 *
 * @author          Hauke Petersen <hauke.peterse@fu-berlin.de>
 */

#ifndef CPU_PERIPH_H
#define CPU_PERIPH_H

#include "periph_cpu_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Redefinition of some register names for unification purposes
 */
#define GPIO_BASE           (NRF_GPIO)
#define UART_IRQN           (UART0_IRQn)
#define SPI_SCKSEL          (dev(bus)->PSELSCK)
#define SPI_MOSISEL         (dev(bus)->PSELMOSI)
#define SPI_MISOSEL         (dev(bus)->PSELMISO)
/** @} */


#endif /* CPU_PERIPH_COMMON_H */
/** @} */
