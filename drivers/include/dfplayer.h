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
 * # About
 *
 * This mod
 *
 * For general information about this module see
 * https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299
 *
 * For additional (more complete) information please also refer to
 * http://www.picaxe.com/docs/spe035.pdf
 *
 * @note    All available documentation for this module seems to be faulty and
 *          incomplete, so the truth must lay somewhat in the middle. Use the
 *          information you find online with care!
 *
 * # Driver design
 *
 * # Features
 *
 * ## Play Songs
 *
 * ## Play Advertisements
 *
 * # Limitations
 * - no support for reading the busy pin -> not really needed as the same
 *   information is available when receiving the CMD_FINISH_x command
 *
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

/**
 * @brief   Size of command packets exchanged with the device (in bytes)
 */
#define DFPLAYER_PKTLEN             (10U)

/**
 * @brief   Default baudrate used by DFPlayer Mini devices
 */
#define DFPLAYER_BAUDRATE_DEFAULT   (9600U)

#ifndef DFPLAYER_PRIO
/**
 * @brief   Priority used for the dedicated driver thread
 */
#define DFPLAYER_PRIO               (THREAD_PRIORITY_MAIN - 2)
#endif

#ifndef DFPLAYER_STACKSIZE
/**
 * @brief   Stacksize used by the dedicated driver thread
 */
#define DFPLAYER_STACKSIZE          (THREAD_STACKSIZE_DEFAULT)
#endif

#ifndef DFPLAYER_TIMEOUT
#define DFPLAYER_TIMEOUT            (500U * US_PER_MS)
#endif

/**
 * @brief   Maximum volume level that can be set
 */
#define DFPLAYER_VOL_MAX            (30U)

/**
 * @brief   Return codes used by the driver
 */
enum {
    DFPLAYER_OK          =  0,
    DFPLAYER_ERR_UART    = -1,
    DFPLAYER_ERR_TIMEOUT = -2,
};

/**
 * @brief   Possible equalizer settings
 */
typedef enum {
    DFPLAYER_NORMAL     = 0,        /**< normal setting */
    DFPLAYER_POP        = 1,        /**< pop setting */
    DFPLAYER_ROCK       = 2,        /**< rock setting */
    DFPLAYER_JAZZ       = 3,        /**< jazz setting */
    DFPLAYER_CLASSIC    = 4,        /**< classic setting */
    DFPLAYER_BASE       = 5,        /**< base setting */
} dfplayer_eq_t;

/**
 * @brief   Playback modes
 */
typedef enum {
    DFPLAYER_REPEAT     = 0,        /**< repeat */
    DFPLAYER_REPEAT_F   = 1,        /**< folder repeat */
    DFPLAYER_REPEAT_S   = 2,        /**< single repeat */
    DFPLAYER_RANDOM     = 3,        /**< play random track */
} dfplayer_mode_t;

/**
 * @brief   Device descriptor type definition
 */
typedef struct dfplayer dfplayer_t;

/**
 * @brief   Custom event type used for event signaling by the driver
 */
typedef struct {
    event_t super;                  /**< we extend the base event */
    dfplayer_t *dev;                /**< device that triggered the event */
    uint16_t param;                 /**< param send with the reply */
    uint8_t code;                   /**< type of message */
} dfplayer_event_t;

/**
 * @brief   Static device configuration parameters
 */
typedef struct {
    uart_t uart;                    /**< UART the device is connected to */
    uint32_t baudrate;              /**< baudrate to use for communication */
} dfplayer_params_t;

/**
 * @brief   Device descriptor definition
 */
struct dfplayer {
    uart_t uart;                    /**< UART device used */
    mutex_t lock;                   /**< mutex for synchronizing API calls */
    uint8_t rx_pos;                 /**< write pointer into RX buffer */
    uint8_t rx_buf[DFPLAYER_PKTLEN];      /**< receive command buffer */
    uint16_t rx_data;               /**< parameter received with reply */
    uint8_t exp_code;               /**< expected response code */
    dfplayer_event_t async_event;   /**< event used for notifications */


    thread_t *waiter;
    xtimer_t to_timer;


    uint32_t time;
};

/* ok */
int dfplayer_init(dfplayer_t *dev, const dfplayer_params_t *params);

/* ok */
int dfplayer_reset(dfplayer_t *dev);

/* ok */
void dfplayer_standby(dfplayer_t *dev);

/* ok */
void dfplayer_wakeup(dfplayer_t *dev);

/* ok */
int dfplayer_ver(dfplayer_t *dev);

/* ok */
int dfplayer_status(dfplayer_t *dev);

/* ok */
void dfplayer_vol_set(dfplayer_t *dev, unsigned level);

/* ok */
void dfplayer_vol_up(dfplayer_t *dev);

/* ok */
void dfplayer_vol_down(dfplayer_t *dev);

/* ok */
int dfplayer_vol_get(dfplayer_t *dev);


void dfplayer_eq_set(dfplayer_t *dev, dfplayer_eq_t eq);

int dfplayer_eq_get(dfplayer_t *dev);

int dfplayer_mode_get(dfplayer_t *dev);

void dfplayer_resume(dfplayer_t *dev);

void dfplayer_play_track(dfplayer_t *dev, unsigned track);

/**
 * @brief      { function_description }
 *
 * Will simply return and do nothing on invalid parameters
 *
 * @param      dev     The development
 * @param[in]  folder  The folder
 * @param[in]  track   The track
 */
void dfplayer_play_folder(dfplayer_t *dev, unsigned folder, unsigned track);

void dfplayer_loop_track(dfplayer_t *dev, unsigned track);

void dfplayer_play_random(dfplayer_t *dev);

void dfplayer_pause(dfplayer_t *dev);

void dfplayer_stop(dfplayer_t *dev);

void dfplayer_next(dfplayer_t *dev);

void dfplayer_prev(dfplayer_t *dev);

void dfplayer_adv_play(dfplayer_t *dev, unsigned track);

void dfplayer_adv_stop(dfplayer_t *dev);

int dfplayer_current_track(dfplayer_t *dev);

int dfplayer_count_files(dfplayer_t *dev);

int dfplayer_count_files_in_folder(dfplayer_t *dev, unsigned folder);



#ifdef __cplusplus
}
#endif

#endif /* DFPLAYER_H */
/** @} */
