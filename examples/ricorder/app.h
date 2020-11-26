/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    cwa-tracker Corona Warn App Tracker
 * @brief       Corona-Warn-App Tracker
 *
 * @{
 * @file
 * @brief       Corona-Warn-App Tracker
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef APP_H
#define APP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCANNER_ITVL        BLE_GAP_SCAN_ITVL_MS(60)
#define SCANNER_WIN         BLE_GAP_SCAN_WIN_MS(60)

typedef struct {
    uint32_t pkt_cnt;
    uint32_t cwa_cnt;
} scanner_stats_t;

int wallclock_init(void);
void wallclock_now(uint32_t *secs, uint32_t *msecs);
struct tm *wallclock_time(void);

int scanner_init(void);
void scanner_getcount(scanner_stats_t *stats);

int stor_init(void);
int stor_write_ln(char *line, size_t len);
void stor_flush(void);

void ui_init(void);
void ui_boot_msg(const char *msg);
void ui_update(void);


#ifdef __cplusplus
}
#endif

#endif /* APP_H */
/** @} */
