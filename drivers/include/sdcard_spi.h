/*
 * Copyright (C) 2016 Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers
 * @brief       Low-level driver for reading and writing sd-cards via spi interface.
 * @{
 *
 * @file
 * @brief       Low-level driver for reading and writing sd-cards via spi interface.
 *              For details of the sd card standard and the spi mode refer to
 *              "SD Specifications Part 1 Physical Layer Simplified Specification".
 *              References to the sd specs in this file apply to Version 5.00
 *              from August 10, 2016. For further details see
 *              https://www.sdcard.org/downloads/pls/pdf/part1_500.pdf.
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


/* number of clocks that should be applied to the card on init
   before taking furter actions (see sd spec. 6.4.1.1 Power Up Time of Card) */
#define SD_POWERSEQUENCE_CLOCK_COUNT 74

#define SD_CARD_PREINIT_CLOCK_PERIOD_US 10 /* used to generate 100 kHz clock in init phase*/
#define SD_CARD_WAIT_AFTER_POWER_UP_US  1000

/* R1 response bits (see sd spec. 7.3.2.1 Format R1) */
#define SD_R1_RESPONSE_PARAM_ERROR       0b01000000
#define SD_R1_RESPONSE_ADDR_ERROR        0b00100000
#define SD_R1_RESPONSE_ERASE_SEQ_ERROR   0b00010000
#define SD_R1_RESPONSE_CMD_CRC_ERROR     0b00001000
#define SD_R1_RESPONSE_ILLEGAL_CMD_ERROR 0b00000100
#define SD_R1_RESPONSE_ERASE_RESET       0b00000010
#define SD_R1_RESPONSE_IN_IDLE_STATE     0b00000001
#define SD_INVALID_R1_RESPONSE           0b10000000

#define R1_VALID(X) (((X) >> 7) == 0)
#define R1_PARAM_ERR(X)   ((((X) &SD_R1_RESPONSE_PARAM_ERROR) != 0))
#define R1_ADDR_ERR(X)    ((((X) &SD_R1_RESPONSE_ADDR_ERROR) != 0))
#define R1_ERASE_ERR(X)   ((((X) &SD_R1_RESPONSE_ERASE_SEQ_ERROR) != 0))
#define R1_CMD_CRC_ERR(X) ((((X) &SD_R1_RESPONSE_CMD_CRC_ERROR) != 0))
#define R1_ILL_CMD_ERR(X) ((((X) &SD_R1_RESPONSE_ILLEGAL_CMD_ERROR) != 0))
#define R1_IDLE_BIT_SET(X) (((X) &SD_R1_RESPONSE_IN_IDLE_STATE) != 0)
#define R1_ERROR(X) (R1_PARAM_ERR(X) || R1_ADDR_ERR(X) || R1_ERASE_ERR(X) || \
                     R1_CMD_CRC_ERR(X) || R1_ILL_CMD_ERR(X))

/* see sd spec. 7.3.3.1 Data Response Token */
#define DATA_RESPONSE_IS_VALID(X)  (((X) & 0b00010001) == 0b00000001)
#define DATA_RESPONSE_ACCEPTED(X)  (((X) & 0b00001110) == 0b00000100)
#define DATA_RESPONSE_CRC_ERR(X)  (((X) & 0b00001110) == 0b00001010)
#define DATA_RESPONSE_WRITE_ERR(X) (((X) & 0b00001110) == 0b00001100)

/* see sd spec. 5.1 OCR register */
#define OCR_VOLTAGE_3_2_TO_3_3 (1 << 20)
#define OCR_VOLTAGE_3_3_TO_3_4 (1 << 21)

/* card capacity status (CCS=0: the card is SDSD; CCS=1: card is SDHC or SDXC) */
#define OCR_CCS (1 << 30)

/* This bit is set to low if the card has not finished power up routine */
#define OCR_POWER_UP_STATUS (1 << 31)

/* to ensure the voltage range check on init is done properly you need to
   define this according to your actual interface/wiring with the sd-card */
#define SYSTEM_VOLTAGE (OCR_VOLTAGE_3_2_TO_3_3 | OCR_VOLTAGE_3_2_TO_3_3)


/* see sd spec. 7.3.1.3 Detailed Command Description */
#define SD_CMD_PREFIX_MASK 0b01000000

#define SD_CMD_0 0   /* Resets the SD Memory Card */
#define SD_CMD_1 1   /* Sends host capacity support info and starts the cards init process */
#define SD_CMD_8 8   /* Sends SD Card interface condition incl. host supply voltage info */
#define SD_CMD_9 9   /* Asks the selected card to send its card-specific data (CSD) */
#define SD_CMD_10 10 /* Asks the selected card to send its card identification (CID) */
#define SD_CMD_12 12 /* Forces the card to stop transmission in Multiple Block Read Operation */
#define SD_CMD_13 13 /* Sent as ACMD13 asks the card to send it's SD status */

#define SD_CMD_16 16 /* In case of SDSC Card, block length is set by this command */
#define SD_CMD_17 17 /* Reads a block of the size selected by the SET_BLOCKLEN command */
#define SD_CMD_18 18 /* Continuously transfers data blocks from card to host
                            until interrupted by a STOP_TRANSMISSION command */
#define SD_CMD_24 24 /* Writes a block of the size selected by the SET_BLOCKLEN command */
#define SD_CMD_25 25 /* Continuously writes blocks of data until 'Stop Tran'token is sent */
#define SD_CMD_41 41 /* Reserved (used for ACMD41) */
#define SD_CMD_55 55 /* Defines to the card that the next commmand is an application specific
                            command rather than a standard command */
#define SD_CMD_58 58 /* Reads the OCR register of a card */
#define SD_CMD_59 59 /* Turns the CRC option on or off. Argument: 1:on; 0:off */

#define SD_CMD_8_VHS_2_7_V_TO_3_6_V 0b00000001
#define SD_CMD_8_CHECK_PATTERN      0b10110101
#define SD_CMD_NO_ARG   0x00000000
#define SD_ACMD_41_ARG_HC 0x40000000
#define SD_CMD_59_ARG_EN  0x00000001
#define SD_CMD_59_ARG_DIS 0x00000000

/* see sd spec. 7.3.3 Control Tokens */
#define SD_DATA_TOKEN_CMD_17_18_24 0b11111110
#define SD_DATA_TOKEN_CMD_25       0b11111100
#define SD_DATA_TOKEN_CMD_25_STOP  0b11111101

#define SD_SIZE_OF_CID_AND_CSD_REG 16
#define SD_SIZE_OF_SD_STATUS 64
#define SD_BLOCKS_FOR_REG_READ 1
#define SD_GET_CSD_STRUCTURE(CSD_RAW_DATA) ((CSD_RAW_DATA)[0] >> 6)
#define SD_CSD_V1 0
#define SD_CSD_V2 1
#define SD_CSD_VUNSUPPORTED -1

#define SD_SIZE_OF_OID 2 /* OID (OEM/application ID field in CID reg) */
#define SD_SIZE_OF_PNM 5 /* PNM (product name field in CID reg) */

/* the retry counters below are used as timeouts for specific actions.
   The values may need some adjustments to either give the card more time to respond
   to commands or to achieve a lower delay / avoid infinite blocking. */
#define R1_POLLING_RETRY_CNT    1000000
#define SD_DATA_TOKEN_RETRY_CNT 1000000
#define INIT_CMD_RETRY_CNT      1000
#define INIT_CMD0_RETRY_CNT     3
#define SD_WAIT_FOR_NOT_BUSY_CNT 10000 /* use -1 for full blocking till the card isn't busy */
#define SD_BLOCK_READ_CMD_RETRIES 10   /* only affects sending of cmd not the whole transaction! */
#define SD_BLOCK_WRITE_CMD_RETRIES 10  /* only affects sending of cmd not the whole transaction! */

#define SD_HC_BLOCK_SIZE 512

/* memory capacity in bytes = (C_SIZE+1) * SD_CSD_V2_C_SIZE_BLOCK_MULT * BLOCK_LEN */
#define SD_CSD_V2_C_SIZE_BLOCK_MULT 1024

#define SD_CARD_SPI_MODE SPI_CONF_FIRST_RISING

/* this speed setting is only used while the init procedure is performed */
#define SD_CARD_SPI_SPEED_PREINIT SPI_SPEED_400KHZ

/* after init procedure is finished the driver auto sets the card to this speed */
#define SD_CARD_SPI_SPEED_POSTINIT SPI_SPEED_10MHZ

#define SD_CARD_DUMMY_BYTE 0xFF

#ifndef NUM_OF_SD_CARDS
    #define NUM_OF_SD_CARDS 1
#endif

typedef enum {
    SD_V2,
    SD_V1,
    MMC_V3,
    SD_UNKNOWN
} sd_version_t;

typedef enum {
    SD_INIT_START,
    SD_INIT_SPI_POWER_SEQ,
    SD_INIT_SEND_CMD0,
    SD_INIT_SEND_CMD8,
    SD_INIT_CARD_UNKNOWN,
    SD_INIT_SEND_ACMD41_HCS,
    SD_INIT_SEND_ACMD41,
    SD_INIT_SEND_CMD1,
    SD_INIT_SEND_CMD58,
    SD_INIT_SEND_CMD16,
    SD_INIT_ENABLE_CRC,
    SD_INIT_READ_CID,
    SD_INIT_READ_CSD,
    SD_INIT_SET_MAX_SPI_SPEED,
    SD_INIT_FINISH
} sd_init_fsm_state_t;

typedef enum {
    SD_RW_OK = 0,
    SD_RW_NO_TOKEN,
    SD_RW_TIMEOUT,
    SD_RW_RX_TX_ERROR,
    SD_RW_WRITE_ERROR,
    SD_RW_CRC_MISMATCH,
    SD_RW_NOT_SUPPORTED
} sd_rw_response_t;

struct {
    uint8_t MID;              /* Manufacturer ID */
    char OID[SD_SIZE_OF_OID]; /* OEM/Application ID*/
    char PNM[SD_SIZE_OF_PNM]; /* Product name */
    uint8_t PRV;              /* Product revision */
    uint32_t PSN;             /* Product serial number */
    uint16_t MDT;             /* Manufacturing date */
    uint8_t CID_CRC;              /* CRC7 checksum */
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

struct {
    spi_t spi_dev;
    gpio_t cs_pin;
    gpio_t clk_pin;
    gpio_t mosi_pin;
    gpio_t miso_pin;
    gpio_t power_pin;
    bool power_pin_act_high;
    bool use_block_addr;
    bool init_done;
    sd_version_t card_type;
    int csd_structure;
    cid_t cid;
    csd_t csd;
} typedef sd_card_t;


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
 * @brief                 Sends a cmd to the sd card.
 *
 * @param[in] card        Initialized sd-card struct
 * @param[in] sd_cmd_idx  A supported sd-card command index for SPI-mode like defined in
 *                        "7.3.1.3 Detailed Command Description" of sd spec.
 *                        (for CMD<X> this parameter is simply the integer value <X>).
 * @param[in] argument    The argument for the given cmd. As described by "7.3.1.1 Command Format".
 *                        This argument is transmitted byte wise with most significant byte first.
 * @param[in] max_retry   Specifies how often the command should be retried if an error occures.
 *                        Use 0 to try only once, -1 to try forever, or n to retry n times.
 *
 * @return                R1 response of the command if no (low-level) communication error occured
 * @return                SD_INVALID_R1_RESPONSE if either waiting for the card to enter
 *                        not-busy-state timed out or spi communication failed
 */
char sdcard_spi_send_cmd(sd_card_t *card, char sd_cmd_idx, uint32_t argument, int max_retry);

/**
 * @brief                 Sends an acmd to the sd card. ACMD<n> consists of sending CMD55 + CMD<n>
 *
 * @param[in] card        Initialized sd-card struct
 * @param[in] sd_cmd_idx  A supported sd-card command index for SPI-mode like defined in
 *                        "7.3.1.3 Detailed Command Description" of sd spec.
 *                        (for ACMD<X> this parameter is simply the integer value <X>).
 * @param[in] argument    The argument for the given cmd. As described by "7.3.1.1 Command Format".
 *                        This argument is transmitted byte wise with most significant byte first.
 * @param[in] max_retry   Specifies how often the command should be retried if an error occures.
 *                        Use 0 to try only once, -1 to try forever, or n to retry n times.
 *
 * @return                R1 response of the command if no (low-level) communication error occured
 * @return                SD_INVALID_R1_RESPONSE if either waiting for the card to enter
 *                        not-busy-state timed out or spi communication failed
 */
char sdcard_spi_send_acmd(sd_card_t *card, char sd_cmd_idx, uint32_t argument, int max_retry);

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

/**
 * @brief                 Gets the sector count of the card.
 *
 * @param[in] card        Initialized sd-card struct
 *
 * @return                number of available sectors
 */
uint32_t sdcard_spi_get_sector_count(sd_card_t *card);

/**
 * @brief                 Gets the allocation unit size of the card.
 *
 * @param[in] card        Initialized sd-card struct
 *
 * @return                size of AU in bytes
 */
uint32_t sdcard_spi_get_au_size(sd_card_t *card);

/**
 * @brief                 Gets the SD status of the card.
 *
 * @param[in] card        Initialized sd-card struct
 *
 * @return                sd_status_t struct that contains all SD status information
 */
sd_rw_response_t sdcard_spi_read_sds(sd_card_t *card, sd_status_t *sd_status);


#ifdef __cplusplus
}
#endif

#endif /* SDCARD_SPI_H */
/** @} */
