/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     app
 * @brief       App specific functions and configuration
 * @{
 *
 * @file
 * @brief       Some App specifics
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SENSOR_DATA_T_TEMP,
    SENSOR_DATA_T_HUM,
    SENSOR_DATA_T_UINT16
} appdata_type_t;

typedef struct {
    uint16_t raw;
    appdata_type_t type;
} appdata_t;

int sense_init(void);
int sense_hwtest(void);
int sense_readall(appdata_t *data);

uint8_t lora_join(void);
void lora_send_data(const appdata_t *data);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */
