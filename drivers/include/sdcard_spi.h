/*
 * Copyright (C) 2016 Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_sdcard_spi SPI SD-Card driver
 * @ingroup     drivers
 * @brief       Driver for reading and writing sd-cards via spi interface.
 * @{
 *
 * @file
 * @brief       Public interface for the sdcard_spi driver.
 *
 * @author      Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 */

#ifndef SDCARD_SPI_H
#define SDCARD_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "periph/spi.h"
#include "periph/gpio.h"
#include "stdbool.h"
#include "sdcard_spi_internal.h"

#define SD_HC_BLOCK_SIZE 512

#ifndef NUM_OF_SD_CARDS
    #define NUM_OF_SD_CARDS 1
#endif

/**
 * @brief           Initializes the sd-card with the given parameters in sd_card_t structure.
 *                  The init procedure also takes care of initializing the spi peripheral to master
 *                  mode and performing all neccecary steps to set the sd-card to spi-mode. Reading
 *                  the CID and CSD registers is also done within this routine and their
 *                  values are copied to the given sd_card_t struct.
 *
 * @param[in] card  struct that contains the pre-set spi_dev (e.g. SPI_1) and cs_pin that are
 *                  connected to the sd card.
 *                  Initialisation of spi_dev and cs_pin are done within this driver.
 *
 * @return          true if the card could be initialized successfully
 * @return          false if an error occured while initializing the card
 */
bool sdcard_spi_init(sd_card_t *card);

/**
 * @brief                 Reads data blocks (usually multiples of 512 Bytes) from card to buffer.
 *
 * @param[in] card        Initialized sd-card struct
 * @param[in] blockaddr   Start adress to read from. Independet of the actual adressing scheme of
 *                        the used card the adress needs to be given as block address
 *                        (e.g. 0, 1, 2... NOT: 0, 512... ). The driver takes care of mapping to
 *                        byte adressing if needed.
 * @param[out] data       Buffer to store the read data in. The user is responsible for providing a
 *                        suitable buffer size.
 * @param[in]  blocksize  Size of data blocks. For now only 512 byte blocks are supported because
 *                        only older (SDSC) cards support variable blocksizes anyway.
 *                        With SDHC/SDXC-cards this is always fixed to 512 bytes. SDSC cards are
 *                        automatically forced to use 512 byte as blocksize by the init procedure.
 * @param[in]  nblocks    Number of blocks to read
 * @param[out] state      Contains information about the error state if something went wrong
 *                        (if return value is lower than nblocks).
 *
 * @return                number of sucessfully read blocks (0 if no block was read).
 */
int sdcard_spi_read_blocks(sd_card_t *card, int blockaddr, char *data, int blocksize, int nblocks, sd_rw_response_t *state);

/**
 * @brief                 Writes data blocks (usually multiples of 512 Bytes) from buffer to card.
 *
 * @param[in] card        Initialized sd-card struct
 * @param[in] blockaddr   Start adress to read from. Independet of the actual adressing scheme of
 *                        the used card the adress needs to be given as block address
 *                        (e.g. 0, 1, 2... NOT: 0, 512... ). The driver takes care of mapping to
 *                        byte adressing if needed.
 * @param[out] data       Buffer that contains the data to be sent.
 * @param[in]  blocksize  Size of data blocks. For now only 512 byte blocks are supported because
 *                        only older (SDSC) cards support variable blocksizes anyway.
 *                        With SDHC/SDXC-cards this is always fixed to 512 bytes. SDSC cards are
 *                        automatically forced to use 512 byte as blocksize by the init procedure.
 * @param[in]  nblocks    Number of blocks to write
 * @param[out] state      Contains information about the error state if something went wrong
 *                         (if return value is lower than nblocks).
 *
 * @return                number of sucessfully written blocks (0 if no block was written).
 */
int sdcard_spi_write_blocks(sd_card_t *card, int blockaddr, char *data, int blocksize, int nblocks, sd_rw_response_t *state);

/**
 * @brief                 Gets the capacity of the card.
 *
 * @param[in] card        Initialized sd-card struct
 *
 * @return                capacity of the card in bytes
 */
uint64_t sdcard_spi_get_capacity(sd_card_t *card);

#ifdef __cplusplus
}
#endif

#endif /* SDCARD_SPI_H */
/** @} */
