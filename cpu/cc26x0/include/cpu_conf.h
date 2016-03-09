/*
 * Copyright (C) 2016 Leon George
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */
/**
 * @addtogroup      cpu_cc26x0
 * @{
 *
 * @file
 * @brief           Implementation specific CPU configuration options
 *
 */

#ifndef CPU_CONF_H
#define CPU_CONF_H

#include "cpu_conf_common.h"

#include <cc26x0.h>

#include "cc26x0-ioc.h"
#include "cc26x0-gpio.h"
#include "cc26x0-prcm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   ARM Cortex-M specific CPU configuration
 * @{
 */
#define CPU_DEFAULT_IRQ_PRIO            (1U)
#define CPU_IRQ_NUMOF                   IRQN_COUNT
#define CPU_FLASH_BASE                  FLASH_BASE
/** @} */

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* CPU_CONF_H */
/** @} */
