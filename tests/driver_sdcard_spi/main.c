/*
 * Copyright (C) 2016 Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for the sd-card spi driver
 *
 * @author      Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 *
 * @}
 */

#ifndef TEST_SDCARD_SPI
#error "TEST_SDCARD_SPI not defined"
#endif
#ifndef TEST_SDCARD_CS
#error "TEST_SDCARD_CS not defined"
#endif

#include "shell.h"
#include "sdcard_spi.h"
#include "fmt.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* independent of what you specify in a r/w cmd this is the maximum number of blocks read at once.
   If you call read with a bigger blockcount the read is performed in chunks*/
#define MAX_BLOCKS_IN_BUFFER 4              
#define BLOCK_PRINT_BYTES_PER_LINE 16
#define FIRST_PRINTABLE_ASCII_CHAR 0x20
#define ASCII_UNPRINTABLE_REPLACEMENT "."

/* this is provided by the sdcard_spi driver see sdcard_spi.c*/
extern sd_card_t cards[NUM_OF_SD_CARDS];
sd_card_t *card = &cards[0];

char buffer[SD_HC_BLOCK_SIZE * MAX_BLOCKS_IN_BUFFER];

static int _init(int argc, char **argv)
{
    printf("Initializing SD-card at SPI_%i...", card->spi_dev);

    if (sdcard_spi_init(card)) {
        printf("[OK]\n");
        return 0;
    }
    else {
        printf("[FAILED]\n");
        #if ENABLE_DEBUG != 1
        printf("enable debugging in sdcard_spi.c for further information!\n");
        #endif
        return -2;
    }

}

static int _cid(int argc, char **argv)
{
    printf("----------------------------------------\n");
    printf("MID: %d\n", card->cid.MID);
    printf("OID: %c%c\n", card->cid.OID[0], card->cid.OID[1]);
    printf("PNM: %c%c%c%c%c\n", card->cid.PNM[0], card->cid.PNM[1], card->cid.PNM[2],
                                card->cid.PNM[3], card->cid.PNM[4]);
    printf("PRV: %d\n", card->cid.PRV);
    printf("PSN: %lu\n", card->cid.PSN);
    printf("MDT: %d\n", card->cid.MDT);
    printf("CRC: %d\n", card->cid.CRC);
    printf("----------------------------------------\n");
    return 0;
}

static int _csd(int argc, char **argv)
{
    if (card->csd_structure == SD_CSD_V1) {
        printf("CSD V1\n----------------------------------------\n");
        printf("CSD_STRUCTURE: 0x%08x\n", card->csd.v1.CSD_STRUCTURE);
        printf("TAAC: 0x%08x\n", card->csd.v1.TAAC);
        printf("NSAC: 0x%08x\n", card->csd.v1.NSAC);
        printf("TRAN_SPEED: 0x%08x\n", card->csd.v1.TRAN_SPEED);
        printf("CCC: 0x%08x\n", card->csd.v1.CCC);
        printf("READ_BL_LEN: 0x%08x\n", card->csd.v1.READ_BL_LEN);
        printf("READ_BL_PARTIAL: 0x%08x\n", card->csd.v1.READ_BL_PARTIAL);
        printf("WRITE_BLK_MISALIGN: 0x%08x\n", card->csd.v1.WRITE_BLK_MISALIGN);
        printf("READ_BLK_MISALIGN: 0x%08x\n", card->csd.v1.READ_BLK_MISALIGN);
        printf("DSR_IMP: 0x%08x\n", card->csd.v1.DSR_IMP);
        printf("C_SIZE: 0x%08x\n", card->csd.v1.C_SIZE);
        printf("VDD_R_CURR_MIN: 0x%08x\n", card->csd.v1.VDD_R_CURR_MIN);
        printf("VDD_R_CURR_MAX: 0x%08x\n", card->csd.v1.VDD_R_CURR_MAX);
        printf("VDD_W_CURR_MIN: 0x%08x\n", card->csd.v1.VDD_W_CURR_MIN);
        printf("VDD_W_CURR_MAX: 0x%08x\n", card->csd.v1.VDD_W_CURR_MAX);
        printf("C_SIZE_MULT: 0x%08x\n", card->csd.v1.C_SIZE_MULT);
        printf("ERASE_BLK_EN: 0x%08x\n", card->csd.v1.ERASE_BLK_EN);
        printf("SECTOR_SIZE: 0x%08x\n", card->csd.v1.SECTOR_SIZE);
        printf("WP_GRP_SIZE: 0x%08x\n", card->csd.v1.WP_GRP_SIZE);
        printf("WP_GRP_ENABLE: 0x%08x\n", card->csd.v1.WP_GRP_ENABLE);
        printf("R2W_FACTOR: 0x%08x\n", card->csd.v1.R2W_FACTOR);
        printf("WRITE_BL_LEN: 0x%08x\n", card->csd.v1.WRITE_BL_LEN);
        printf("WRITE_BL_PARTIAL: 0x%08x\n", card->csd.v1.WRITE_BL_PARTIAL);
        printf("FILE_FORMAT_GRP: 0x%08x\n", card->csd.v1.FILE_FORMAT_GRP);
        printf("COPY: 0x%08x\n", card->csd.v1.COPY);
        printf("PERM_WRITE_PROTECT: 0x%08x\n", card->csd.v1.PERM_WRITE_PROTECT);
        printf("TMP_WRITE_PROTECT: 0x%08x\n", card->csd.v1.TMP_WRITE_PROTECT);
        printf("FILE_FORMAT: 0x%08x\n", card->csd.v1.FILE_FORMAT);
        printf("CRC: 0x%08x\n", card->csd.v1.CRC);
    }
    else if (card->csd_structure == SD_CSD_V2) {
        printf("CSD V2:\n----------------------------------------\n");
        printf("CSD_STRUCTURE: 0x%08x\n", card->csd.v2.CSD_STRUCTURE);
        printf("TAAC: 0x%08x\n", card->csd.v2.TAAC);
        printf("NSAC: 0x%08x\n", card->csd.v2.NSAC);
        printf("TRAN_SPEED: 0x%08x\n", card->csd.v2.TRAN_SPEED);
        printf("CCC: 0x%08x\n", card->csd.v2.CCC);
        printf("READ_BL_LEN: 0x%08x\n", card->csd.v2.READ_BL_LEN);
        printf("READ_BL_PARTIAL: 0x%08x\n", card->csd.v2.READ_BL_PARTIAL);
        printf("WRITE_BLK_MISALIGN: 0x%08x\n", card->csd.v2.WRITE_BLK_MISALIGN);
        printf("READ_BLK_MISALIGN: 0x%08x\n", card->csd.v2.READ_BLK_MISALIGN);
        printf("DSR_IMP: 0x%08x\n", card->csd.v2.DSR_IMP);
        printf("C_SIZE: 0x%08x\n", card->csd.v2.C_SIZE);
        printf("ERASE_BLK_EN: 0x%08x\n", card->csd.v2.ERASE_BLK_EN);
        printf("SECTOR_SIZE: 0x%08x\n", card->csd.v2.SECTOR_SIZE);
        printf("WP_GRP_SIZE: 0x%08x\n", card->csd.v2.WP_GRP_SIZE);
        printf("WP_GRP_ENABLE: 0x%08x\n", card->csd.v2.WP_GRP_ENABLE);
        printf("R2W_FACTOR: 0x%08x\n", card->csd.v2.R2W_FACTOR);
        printf("WRITE_BL_LEN: 0x%08x\n", card->csd.v2.WRITE_BL_LEN);
        printf("WRITE_BL_PARTIAL: 0x%08x\n", card->csd.v2.WRITE_BL_PARTIAL);
        printf("FILE_FORMAT_GRP: 0x%08x\n", card->csd.v2.FILE_FORMAT_GRP);
        printf("COPY: 0x%08x\n", card->csd.v2.COPY);
        printf("PERM_WRITE_PROTECT: 0x%08x\n", card->csd.v2.PERM_WRITE_PROTECT);
        printf("TMP_WRITE_PROTECT: 0x%08x\n", card->csd.v2.TMP_WRITE_PROTECT);
        printf("FILE_FORMAT: 0x%08x\n", card->csd.v2.FILE_FORMAT);
        printf("CRC: 0x%08x\n", card->csd.v2.CRC);
    }
    printf("----------------------------------------\n");
    return 0;
}

static int _size(int argc, char **argv)
{
    uint64_t bytes = sdcard_spi_get_capacity(card);

    uint32_t gib_int = bytes / (1024 * 1024 * 1024);
    uint32_t gib_frac = ((((bytes/(1024 * 1024)) - gib_int * 1024) * 1000) / 1024);

    uint32_t gb_int = bytes / (1000000000);
    uint32_t gb_frac = (bytes / (1000000)) - (gb_int * 1000); //[MB]

    printf("\nCard size: ");
    fflush(stdout);
    print_u64_dec( bytes );
    printf(" bytes (%lu,%03lu GiB | %lu,%03lu GB)\n", gib_int, gib_frac, gb_int, gb_frac);
    return 0;
}

static int _read(int argc, char **argv)
{
    int blockaddr;
    int cnt;
    bool print_as_char = false;

    if (argc == 3 || argc == 4) {
        blockaddr = (uint32_t)atoi(argv[1]);
        cnt = (uint32_t)atoi(argv[2]);
        if (argc == 4 && (strcmp("-c", argv[3]) == 0)) {
            print_as_char = true;
        }
    }
    else {
        printf("usage: %s blockaddr cnt [-c]\n", argv[0]);
        return -1;
    }

    int total_read = 0;
    while (total_read < cnt) {
        int chunk_blocks = cnt - total_read;
        if (chunk_blocks > MAX_BLOCKS_IN_BUFFER) {
            chunk_blocks = MAX_BLOCKS_IN_BUFFER;
        }
        sd_rw_response_t state;
        int chunks_read = sdcard_spi_read_blocks(card, blockaddr + total_read, buffer, 
                                                 SD_HC_BLOCK_SIZE, chunk_blocks, &state);

        if (state != SD_RW_OK) {
            printf("read error %d (block %d/%d)\n", state, total_read + chunks_read, cnt);
            return -1;
        }

        for (int i = 0; i < chunk_blocks * SD_HC_BLOCK_SIZE; i++) {

            if (((i % SD_HC_BLOCK_SIZE) == 0)) {
                printf("BLOCK %d:\n", blockaddr + total_read + i / SD_HC_BLOCK_SIZE);
            }

            if (print_as_char) {
                if (buffer[i] >= FIRST_PRINTABLE_ASCII_CHAR) {
                    printf("%c", buffer[i]);
                }
                else {
                    printf(ASCII_UNPRINTABLE_REPLACEMENT);
                }
            }
            else {
                printf("%02x ", buffer[i]);
            }

            if ((i % BLOCK_PRINT_BYTES_PER_LINE) == (BLOCK_PRINT_BYTES_PER_LINE - 1)) {
                printf("\n"); /* line break after BLOCK_PRINT_BYTES_PER_LINE bytes */
            }

            if ((i % SD_HC_BLOCK_SIZE) == (SD_HC_BLOCK_SIZE - 1)) {
                printf("\n"); /* empty line after each printed block */
            }
        }
        total_read += chunks_read;
    }
    return 0;
}

static int _write(int argc, char **argv)
{
    int bladdr;
    char *data;
    int size;
    bool repeat_data = false;

    if (argc == 3 || argc == 4) {
        bladdr = (int)atoi(argv[1]);
        data = argv[2];
        size = strlen(argv[2]);
        printf("will write '%s' (%d chars) at start of block %d\n", data, size, bladdr);
        if (argc == 4 && (strcmp("-r", argv[3]) == 0)) {
            repeat_data = true;
            printf("the rest of the block will be filled with copies of that string\n");
        }
        else {
            printf("the rest of the block will be filled with zeros\n");
        }
    }
    else {
        printf("usage: %s blockaddr string [-r]\n", argv[0]);
        return -1;
    }

    if (size > SD_HC_BLOCK_SIZE) {
        printf("maximum stringsize to write at once is %d ...aborting\n", SD_HC_BLOCK_SIZE);
        return -1;
    }

    /* copy data to a full-block-sized buffer an fill remaining block space according to -r param*/
    char buffer[SD_HC_BLOCK_SIZE];
    for (int i = 0; i < sizeof(buffer); i++) {
        if (repeat_data || i < size) {
            buffer[i] = data[i % size];
        }
        else {
            buffer[i] = 0;
        }
    }

    sd_rw_response_t state;
    int chunks_written = sdcard_spi_write_blocks(card, bladdr, buffer, SD_HC_BLOCK_SIZE, 1, &state);

    if (state != SD_RW_OK) {
        printf("write error %d (wrote %d/%d blocks)\n", state, chunks_written, 1);
        return -1;
    }
    else {
        printf("write block %d [OK]\n", bladdr);
        return 0;
    }
}

static int _copy(int argc, char **argv)
{
    int src_block;
    int dst_block;

    if (argc == 3) {
        src_block = (uint32_t)atoi(argv[1]);
        dst_block = (uint32_t)atoi(argv[2]);
    }
    else {
        printf("usage: %s src_block dst_block\n", argv[0]);
        return -1;
    }

    char tmp_copy[SD_HC_BLOCK_SIZE];

    sd_rw_response_t rd_state;
    sdcard_spi_read_blocks(card, src_block, tmp_copy, SD_HC_BLOCK_SIZE, 1, &rd_state);
    if (rd_state != SD_RW_OK) {
        printf("read error %d (block %d)\n", rd_state, src_block);
        return -1;
    }
    else {
        sd_rw_response_t wr_state;
        sdcard_spi_write_blocks(card, dst_block, tmp_copy, SD_HC_BLOCK_SIZE, 1, &wr_state);

        if (wr_state != SD_RW_OK) {
            printf("write error %d (block %d)\n", wr_state, dst_block);
            return -1;
        }
        else {
            printf("copy block %d to %d [OK]\n", src_block, dst_block);
            return 0;
        }
    }
}

static int _sector_count(int argc, char **argv)
{
    printf("available sectors on card: %li\n", sdcard_spi_get_sector_count(card));
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "init", "initialize card", _init },
    { "cid",  "print content of CID (Card IDentification) register", _cid },
    { "csd", "print content of CSD (Card-Specific Data) register", _csd },
    { "size", "print card size", _size },
    { "sectors", "print sector count of card", _sector_count },
    { "read", "'read n m' reads m blocks beginning at block address n and prints the result. \
Append -c option to print data readable chars", _read },
    { "write", "'write n data' writes data to block n. Append -r option to \
repeatedly write data to coplete block", _write },
    { "copy", "'copy src dst' copies block src to block dst", _copy },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("SD-card spi driver test application\n");
    
    card->spi_dev = TEST_SDCARD_SPI;
    card->cs_pin = TEST_SDCARD_CS;
    card->init_done = false;

    puts("insert SD-card and use 'init' command to set card to spi mode\n");
    puts("WARNING: using 'write' or 'copy' commands WILL overwrite data on your sd-card and\n\
almost for sure corrupt existing filesystems, partitions and contained data!\n");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
