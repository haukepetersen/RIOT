/*
 * Copyright (C) 2015-2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_lpc11u34
 * @{
 *
 * @file
 * @brief           CPU specific definitions for internal peripheral handling
 *
 * @author          Paul RATHGEB <paul.rathgeb@skynet.be>
 * @author          Hauke Petersen<hauke.petersen@fu-berlin.de>
 */

#ifndef PERIPH_CPU_H_
#define PERIPH_CPU_H_

#include "cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief declare needed generic SPI functions
 * @{
 */
#define PERIPH_SPI_NEEDS_TRANSFER_BYTES
#define PERIPH_SPI_NEEDS_TRANSFER_REG
#define PERIPH_SPI_NEEDS_TRANSFER_REGS
/** @} */

/**
 * @brief   Length of the CPU_ID in octets
 */
#define CPUID_LEN           (16U)

/**
 * @brief   Define number of available ADC lines
 *
 * TODO: check this value
 */
#define ADC_NUMOF           (10U)

#ifndef DOXYGEN
/**
 * @brief   Override the ADC resolution settings
 * @{
 */
#define HAVE_ADC_RES_T
typedef enum {
    ADC_RES_6BIT = 0,       /**< ADC resolution: 6 bit */
    ADC_RES_8BIT,           /**< ADC resolution: 8 bit */
    ADC_RES_10BIT,          /**< ADC resolution: 10 bit */
    ADC_RES_12BIT,          /**< ADC resolution: 12 bit */
    ADC_RES_14BIT,          /**< ADC resolution: 14 bit */
    ADC_RES_16BIT,          /**< ADC resolution: 16 bit */
} adc_res_t;
/** @} */
#endif /* ndef DOXYGEN */

/**
 * @brief   Override SPI clock speed values
 *
 * @note    The values expect the CPU to run at 12MHz
 * @todo    Generalize the SPI driver
 */
#define HAVE_SPI_CLK_T
typedef enum {
    SPI_CLK_100KHZ = 119,         /**< drive the SPI bus with 100KHz */
    SPI_CLK_400KHZ =  29,         /**< drive the SPI bus with 400KHz */
    SPI_CLK_1MHZ   =  11,           /**< drive the SPI bus with 1MHz */
    SPI_CLK_5MHZ   =   2,           /**< drive the SPI bus with 5MHz */
    SPI_CLK_10MHZ  =   0        /**< actual: 12 MHz */
} spi_clk_t;


/**
 * @brief   SPI configuration data
 */
typedef struct {
    LPC_SSPx_Type *dev;
    uint32_t preset_bit;
    uint32_t ahb_bit;
} spi_conf_t;

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CPU_H_ */
/** @} */
