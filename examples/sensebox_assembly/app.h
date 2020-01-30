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
} data_type_te;

/*TODO: make this defined in only one place in the code */
typedef struct {
    uint16_t raw;
    data_type_te type;
} data_t;


void s_and_a_init_all(data_t* data);
void s_and_a_hardware_test(void);
void s_and_a_update_all(data_t* data);

uint8_t lora_join(void);
void lora_send_data(data_t *data, int len);


#ifdef __cplusplus
}
#endif

#endif /* APP_H */
