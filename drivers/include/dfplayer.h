/*
 * Copyright (C) 2018 Hauke Petersen <devel@haukepetersen.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_dfplayer DFPlayer Mini Driver
 * @ingroup     drivers_actuators
 * @brief       Driver the DFPlayer Mini audio playback device
 *
 * @{
 * @file
 * @brief       Interface definition for the DFPLayer Mini device driver
 *
 * @author      Hauke Petersen <devel@haukepetersen.de>
 */

#ifndef DFPLAYER_H
#define DFPLAYER_H

#include <stdint.h>
#include <stdbool.h>

#include "event.h"
#include "mutex.h"
#include "xtimer.h"
#include "periph/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DFPLAYER_PKTLEN             (10U)

#define DFPLAYER_BAUDRATE_DEFAULT   (9600U)

#ifndef DFPLAYER_PRIO
#define DFPLAYER_PRIO               (THREAD_PRIORITY_MAIN - 2)
#endif

#ifndef DFPLAYER_STACKSIZE
#define DFPLAYER_STACKSIZE          (THREAD_STACKSIZE_DEFAULT)
#endif

#ifndef DFPLAYER_TIMEOUT
#define DFPLAYER_TIMEOUT            (200U * US_PER_MS)
#endif

#define DFPLAYER_VOL_MIN            (0U)
#define DFPLAYER_VOL_MAX            (31U)

enum {
    DFPLAYER_OK          =  0,
    DFPLAYER_ERR_UART    = -1,
    DFPLAYER_ERR_TIMEOUT = -2,
};

typedef enum {
    DFPLAYER_NORMAL     = 0,
    DFPLAYER_POP        = 1,
    DFPLAYER_ROCK       = 2,
    DFPLAYER_JAZZ       = 3,
    DFPLAYER_CLASSIC    = 4,
    DFPLAYER_BASE       = 5,
} dfplayer_eq_t;

typedef enum {
    DFPLAYER_REPEAT     = 0,        /**< repeat */
    DFPLAYER_REPEAT_F   = 1,        /**< folder repeat */
    DFPLAYER_REPEAT_S   = 2,        /**< single repeat */
    DFPLAYER_RANDOM     = 3,        /**< play random track */
} dfplayer_mode_t;

typedef struct dfplayer dfplayer_t;

typedef struct {
    event_t super;
    dfplayer_t *dev;
    uint16_t param;
    uint8_t code;
    uint8_t flags;
} dfplayer_event_t;

typedef union {
    struct __attribute__((packed)) {
        uint8_t start;
        uint8_t ver;
        uint8_t len;
        uint8_t cmd;
        uint8_t feedback;
        uint8_t param[2];
        uint8_t csum[2];
        uint8_t end;
    } pkt;
    uint8_t raw[DFPLAYER_PKTLEN];
} dfplayer_buf_t;

typedef struct {
    uart_t uart;
    uint32_t baudrate;
} dfplayer_params_t;

struct dfplayer {
    uart_t uart;
    mutex_t lock;
    uint8_t rx_pos;
    dfplayer_buf_t rx_buf;      /**< receive command buffer */
    uint16_t rx_data;
    uint8_t exp_code;
    dfplayer_event_t async_event;


    thread_t *waiter;
    xtimer_t to_timer;


    uint32_t time;
};

int dfplayer_init(dfplayer_t *dev, const dfplayer_params_t *params);

int dfplayer_ver_get(dfplayer_t *dev);

int dfplayer_reset(dfplayer_t *dev);

void dfplayer_standby(dfplayer_t *dev);

void dfplayer_wakeup(dfplayer_t *dev);

void dfplayer_vol_set(dfplayer_t *dev, uint16_t volume);

void dfplayer_vol_up(dfplayer_t *dev);

void dfplayer_vol_down(dfplayer_t *dev);

int dfplayer_vol_get(dfplayer_t *dev);

void dfplayer_eq_set(dfplayer_t *dev, dfplayer_eq_t eq);

int dfplayer_eq_get(dfplayer_t *dev);

void dfplayer_mode_set(dfplayer_t *dev, dfplayer_mode_t mode);

int dfplayer_mode_get(dfplayer_t *dev);

void dfplayer_play(dfplayer_t *dev);

void dfplayer_play_track(dfplayer_t *dev, uint16_t track);

void dfplayer_play_folder(dfplayer_t *dev, uint16_t folder, uint16_t track);

void dfplayer_pause(dfplayer_t *dev);

void dfplayer_repeat_play(dfplayer_t *dev, bool start);

void dfplayer_next(dfplayer_t *dev);

void dfplayer_prev(dfplayer_t *dev);


#ifdef __cplusplus
}
#endif

#endif /* DFPLAYER_H */
/** @} */
