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

#define SD_HC_BLOCK_SIZE      (512)
#define SDCARD_SPI_INIT_ERROR (-1)
#define SDCARD_SPI_OK         (0)

#define SD_SIZE_OF_OID 2 /* OID (OEM/application ID field in CID reg) */
#define SD_SIZE_OF_PNM 5 /* PNM (product name field in CID reg) */

struct {
    uint8_t MID;              /* Manufacturer ID */
    char OID[SD_SIZE_OF_OID]; /* OEM/Application ID*/
    char PNM[SD_SIZE_OF_PNM]; /* Product name */
    uint8_t PRV;              /* Product revision */
    uint32_t PSN;             /* Product serial number */
    uint16_t MDT;             /* Manufacturing date */
    uint8_t CID_CRC;          /* CRC7 checksum */
} typedef cid_t;

/* see sd spec. 5.3.2 CSD Register (CSD Version 1.0) */
struct {
    uint8_t CSD_STRUCTURE : 2;
    uint8_t TAAC : 8;
    uint8_t NSAC : 8;
    uint8_t TRAN_SPEED : 8;
    uint16_t CCC : 12;
    uint8_t READ_BL_LEN : 4;
    uint8_t READ_BL_PARTIAL : 1;
    uint8_t WRITE_BLK_MISALIGN : 1;
    uint8_t READ_BLK_MISALIGN : 1;
    uint8_t DSR_IMP : 1;
    uint16_t C_SIZE : 12;
    uint8_t VDD_R_CURR_MIN : 3;
    uint8_t VDD_R_CURR_MAX : 3;
    uint8_t VDD_W_CURR_MIN : 3;
    uint8_t VDD_W_CURR_MAX : 3;
    uint8_t C_SIZE_MULT : 3;
    uint8_t ERASE_BLK_EN : 1;
    uint8_t SECTOR_SIZE : 7;
    uint8_t WP_GRP_SIZE : 7;
    uint8_t WP_GRP_ENABLE : 1;
    uint8_t R2W_FACTOR : 3;
    uint8_t WRITE_BL_LEN : 4;
    uint8_t WRITE_BL_PARTIAL : 1;
    uint8_t FILE_FORMAT_GRP : 1;
    uint8_t COPY : 1;
    uint8_t PERM_WRITE_PROTECT : 1;
    uint8_t TMP_WRITE_PROTECT : 1;
    uint8_t FILE_FORMAT : 2;
    uint8_t CSD_CRC : 8;
} typedef csd_v1_t;

/* see sd spec. 5.3.3 CSD Register (CSD Version 2.0) */
struct {
    uint8_t CSD_STRUCTURE : 2;
    uint8_t TAAC : 8;
    uint8_t NSAC : 8;
    uint8_t TRAN_SPEED : 8;
    uint16_t CCC : 12;
    uint8_t READ_BL_LEN : 4;
    uint8_t READ_BL_PARTIAL : 1;
    uint8_t WRITE_BLK_MISALIGN : 1;
    uint8_t READ_BLK_MISALIGN : 1;
    uint8_t DSR_IMP : 1;
    uint32_t C_SIZE : 22;
    uint8_t ERASE_BLK_EN : 1;
    uint8_t SECTOR_SIZE : 7;
    uint8_t WP_GRP_SIZE : 7;
    uint8_t WP_GRP_ENABLE : 1;
    uint8_t R2W_FACTOR : 3;
    uint8_t WRITE_BL_LEN : 4;
    uint8_t WRITE_BL_PARTIAL : 1;
    uint8_t FILE_FORMAT_GRP : 1;
    uint8_t COPY : 1;
    uint8_t PERM_WRITE_PROTECT : 1;
    uint8_t TMP_WRITE_PROTECT : 1;
    uint8_t FILE_FORMAT : 2;
    uint8_t CSD_CRC : 8;
} typedef csd_v2_t;

union {
    csd_v1_t v1;
    csd_v2_t v2;
} typedef csd_t;

struct {
    uint32_t SIZE_OF_PROTECTED_AREA : 32;
    uint32_t SUS_ADDR : 22;
    uint32_t VSC_AU_SIZE : 10;
    uint16_t SD_CARD_TYPE : 16;
    uint16_t ERASE_SIZE : 16;
    uint8_t  SPEED_CLASS : 8;
    uint8_t  PERFORMANCE_MOVE : 8;
    uint8_t  VIDEO_SPEED_CLASS : 8;
    uint8_t  ERASE_TIMEOUT : 6;
    uint8_t  ERASE_OFFSET : 2;
    uint8_t  UHS_SPEED_GRADE : 4;
    uint8_t  UHS_AU_SIZE : 4;
    uint8_t  AU_SIZE : 4;
    uint8_t  DAT_BUS_WIDTH : 2;
    uint8_t  SECURED_MODE : 1;
} typedef sd_status_t;

typedef enum {
    SD_V2,
    SD_V1,
    MMC_V3,
    SD_UNKNOWN
} sd_version_t;

typedef enum {
    SD_RW_OK = 0,
    SD_RW_NO_TOKEN,
    SD_RW_TIMEOUT,
    SD_RW_RX_TX_ERROR,
    SD_RW_WRITE_ERROR,
    SD_RW_CRC_MISMATCH,
    SD_RW_NOT_SUPPORTED
} sd_rw_response_t;

/**
 * @brief   sdcard_spi device params
 */
typedef struct {
    spi_t spi_dev;          /**< SPI bus used */
    gpio_t cs;              /**< pin connected to the DAT3 sd pad */
    gpio_t clk;             /**< pin connected to the CLK sd pad */
    gpio_t mosi;            /**< pin connected to the CMD sd pad*/
    gpio_t miso;            /**< pin connected to the DAT0 sd pad*/
    gpio_t power;           /**< pin that controls sd power circuit*/
    bool power_act_high;    /**< true if card power is enabled by 'power'-pin HIGH*/
} sdcard_spi_params_t;

/**
 * @brief   Device descriptor for sdcard_spi
 */
struct {
    sdcard_spi_params_t params;
    bool use_block_addr;
    bool init_done;
    sd_version_t card_type;
    int csd_structure;
    cid_t cid;
    csd_t csd;
} typedef sdcard_spi_t;

/**
 * @brief           Initializes the sd-card with the given parameters in sdcard_spi_t structure.
 *                  The init procedure also takes care of initializing the spi peripheral to master
 *                  mode and performing all neccecary steps to set the sd-card to spi-mode. Reading
 *                  the CID and CSD registers is also done within this routine and their
 *                  values are copied to the given sdcard_spi_t struct.
 *
 * @param[in] card  struct that contains the pre-set spi_dev (e.g. SPI_1) and cs_pin that are
 *                  connected to the sd card.
 *                  Initialisation of spi_dev and cs_pin are done within this driver.
 *
 * @return          0 if the card could be initialized successfully
 * @return          false if an error occured while initializing the card
 */
int sdcard_spi_init(sdcard_spi_t *card, const sdcard_spi_params_t *params);

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
int sdcard_spi_read_blocks(sdcard_spi_t *card, int blockaddr, char *data, int blocksize,
	                       int nblocks, sd_rw_response_t *state);

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
int sdcard_spi_write_blocks(sdcard_spi_t *card, int blockaddr, char *data, int blocksize,
	                        int nblocks, sd_rw_response_t *state);

/**
 * @brief                 Gets the capacity of the card.
 *
 * @param[in] card        Initialized sd-card struct
 *
 * @return                capacity of the card in bytes
 */
uint64_t sdcard_spi_get_capacity(sdcard_spi_t *card);

#ifdef __cplusplus
}
#endif

#endif /* SDCARD_SPI_H */
/** @} */
