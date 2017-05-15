/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    core_cpu Generic CPU interface
 * @ingroup     core
 * @brief       Global CPU interface
 *
 * @{
 * @file
 * @brief       Global CPU interface as base point for ported platforms
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef CPU_H
#define CPU_H

#include "cpu_arch_defs.h"
#include "cpu_arch_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initialize the CPU
 *
 * The initialization prepares and configures typically things like interrupt
 * vectors, clock systems, and peripherals.
 */
void cpu_init(void);

#ifdef __cplusplus
}
#endif

#endif /* CPU_H */
/** @} */
