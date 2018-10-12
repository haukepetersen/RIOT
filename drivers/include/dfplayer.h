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
 * - the driver does not keep track of time, so sending certain commands too
 *   quickly might lead to those commands being ignored
 *
 * # Known-issues
 * - the DFPLAYER_ON_TRACK_FINISH event is triggered twice after a track has
 *   finished its playback
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

#ifndef DFPLAYER_TIMEOUT
/**
 * @brief   Amount of time to wait for valid command reply (in us)
 */
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
    DFPLAYER_OK          =  0,      /**< all ok */
    DFPLAYER_ERR_UART    = -1,      /**< error initializing UART interface */
    DFPLAYER_ERR_TIMEOUT = -2,      /**< command timeout */
};

/**
 * @brief   Asynchronous device events
 */
enum {
    DFPLAYER_ON_MEM_INSERTED = 0x3a,    /**< memory card was inserted */
    DFPLAYER_ON_MEM_EJECTED  = 0x3b,    /**< memory card was removed */
    DFPLAYER_ON_TRACK_FINISH = 0x3d,    /**< finished playback of track */
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
 * @brief   Event callback type definition
 *
 * @warning Event callbacks are executed in interrupt context. You can not call
 *          any DFPlayer API functions directly from within that context!
 */
typedef void(*dfplayer_event_cb_t)(dfplayer_t *dev, uint8_t e, unsigned p);

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
    uart_t uart;                        /**< UART device used */
    mutex_t lock;                       /**< mutex for synchronizing API calls */
    uint8_t rx_pos;                     /**< write pointer into RX buffer */
    uint8_t rx_buf[DFPLAYER_PKTLEN];    /**< receive command buffer */
    uint16_t rx_data;                   /**< parameter received with reply */
    uint8_t exp_code;                   /**< expected response code */
    thread_t *waiter;                   /**< waiter during ongoing operation */
    xtimer_t to_timer;                  /**< timeout timer */
    dfplayer_event_cb_t event_cb;       /**< function called on async events */
};

/**
 * @brief   Initialize the given DFPlayer device
 *
 * @param[in,out] dev       DFPlayer mini device
 * @param[in]     params    device configuration parameters
 *
 * @return  DFPLAYER_OK on success
 * @return  DFPLAYER_ERR_UART if driver fails to initialize UART peripheral
 * @return  DFPLAYER_ERR_TIMEOUT on timeout waiting for reply
 */
int dfplayer_init(dfplayer_t *dev, const dfplayer_params_t *params);

/**
 * @brief   Set the active event callback
 *
 * @param[in,out] dev       DFplayer mini device
 * @param[in]     cb        event callback function, NULL for removing the
 *                          possibly existing entry
 */
void dfplayer_set_event_cb(dfplayer_t *dev, dfplayer_event_cb_t cb);

/**
 * @brief   Reset the given device
 *
 * @param[in,out] dev       DFPlayer mini device
 *
 * @return  DFPLAYER_OK on success
 * @return  DFPLAYER_ERR_TIMEOUT on timeout waiting for reply
 */
int dfplayer_reset(dfplayer_t *dev);

/**
 * @brief   Put the given device into standby mode
 *
 * @param[in,out] dev       DFPlayer mini device
 */
void dfplayer_standby(dfplayer_t *dev);

/**
 * @brief   Wakeup the given device from standby
 *
 * @note    There seem to be a number of DFPlayer Mini modules on the market
 *          that do not wake up from standby properly...
 *
 * @param[in,out] dev       DFPlayer mini device
 */
void dfplayer_wakeup(dfplayer_t *dev);

/**
 * @brief   Get the version number of the given device
 *
 * @param[in,out] dev       DFPlayer mini device
 *
 * @return  version number of the given device
 * @return  DFPLAYER_ERR_TIMEOUT on timeout waiting for reply
 */
int dfplayer_ver(dfplayer_t *dev);

/**
 * @brief   Get the current device status
 *
 * @param[in,out] dev       DFPlayer mini device
 *
 * @return  status of the given device
 * @return  DFPLAYER_ERR_TIMEOUT on timeout waiting for reply
 */
int dfplayer_status(dfplayer_t *dev);

/**
 * @brief   Set the volume level ([0-30])
 *
 * Setting the volume level to any value to anything larger than 30 will just
 * set it to 30.
 *
 * @param[in,out] dev       DFPlayer mini device
 * @param[in]     level     new volume level ([0-30])
 */
void dfplayer_vol_set(dfplayer_t *dev, unsigned level);

/**
 * @brief   Increase volume level by 1
 *
 * @param[in,out] dev       DFPlayer mini device
 */
void dfplayer_vol_up(dfplayer_t *dev);

/**
 * @brief   Decrease volume level by 1
 *
 * @param[in,out] dev       DFPlayer mini device
 */
void dfplayer_vol_down(dfplayer_t *dev);

/**
 * @brief   Get the currently set volume level
 *
 * @param[in,out] dev       DFPlayer mini device
 *
 * @return  currently set volume level ([0-30])
 * @return  DFPLAYER_ERR_TIMEOUT on timeout waiting for reply
 */
int dfplayer_vol_get(dfplayer_t *dev);

/**
 * @brief   Apply a new equalizer setting
 *
 * @param[in,out] dev       DFPlayer mini device
 * @param[in]     eq        equalizer setting to use
 */
void dfplayer_eq_set(dfplayer_t *dev, dfplayer_eq_t eq);

/**
 * @brief   Get the current equalizer setting
 *
 * @param[in,out] dev       DFPlayer mini device
 *
 * @return  current equalizer setting
 * @return  DFPLAYER_ERR_TIMEOUT on timeout waiting for reply
 */
int dfplayer_eq_get(dfplayer_t *dev);

/**
 * @brief   Get the currently active playback mode
 *
 * @param[in,out] dev       DFPlayer mini device
 *
 * @return  currently active playback mode (value from dfplayer_mode_t)
 * @return  DFPLAYER_ERR_TIMEOUT on timeout waiting for reply
 */
int dfplayer_mode_get(dfplayer_t *dev);

/**
 * @brief   Play the given track (using its global track number)
 *
 * @param[in,out] dev       DFPlayer mini device
 * @param[in]     track     track to play
 */
void dfplayer_play_track(dfplayer_t *dev, unsigned track);

/**
 * @brief   Play a track from the given folder
 *
 * @param[in,out] dev       DFPlayer mini device
 * @param[in]     folder    folder to play from [1-99]
 * @param[in]     track     track number to play [1-255]
 */
void dfplayer_play_folder(dfplayer_t *dev, unsigned folder, unsigned track);

/**
 * @brief   Play the given track in an endless loop (global track number)
 *
 * @param[in,out] dev       DFPlayer mini device
 * @param[in]     track     global track number of track to loop
 */
void dfplayer_loop_track(dfplayer_t *dev, unsigned track);

/**
 * @brief   Start to play tracks in random sequence
 *
 * Using this command will always play the first available track. But any
 * subsequent track will be chosen randomly.
 *
 * @param[in,out] dev       DFPlayer mini device
 */
void dfplayer_play_random(dfplayer_t *dev);

/**
 * @brief   Pause playback
 *
 * @param[in,out] dev       DFPlayer mini device
 */
void dfplayer_pause(dfplayer_t *dev);

/**
 * @brief   Resume playback
 *
 * @param[in,out] dev       DFPlayer mini device
 */
void dfplayer_resume(dfplayer_t *dev);

/**
 * @brief   Stop playback
 *
 * @param[in,out] dev       DFPlayer mini device
 */
void dfplayer_stop(dfplayer_t *dev);

/**
 * @brief   Play next track in list (depending on active playback mode)
 *
 * @param[in,out] dev       DFPlayer mini device
 */
void dfplayer_next(dfplayer_t *dev);

/**
 * @brief   Play previous track
 *
 * @param[in,out] dev       DFPlayer mini device
 */
void dfplayer_prev(dfplayer_t *dev);

/**
 * @brief   Play an advertisement track, interrupting the current playback
 *
 * Advertisement tracks are selected from the `/advert/` sub-folder of the SD
 * card.
 *
 * @param[in,out] dev       DFPlayer mini device
 * @param[in]     track     track number to play
 */
void dfplayer_adv_play(dfplayer_t *dev, unsigned track);

/**
 * @brief   Stop playback of advertisement track
 *
 * @param[in,out] dev       DFPlayer mini device
 */
void dfplayer_adv_stop(dfplayer_t *dev);

/**
 * @brief   Count the number of files in the given folder
 *
 * @param[in,out] dev       DFPlayer mini device
 *
 * @return  the global track number of the current track
 * @return  DFPLAYER_ERR_TIMEOUT on timeout waiting for reply
 */
int dfplayer_current_track(dfplayer_t *dev);

/**
 * @brief   Count the overall number of available tracks
 *
 * @param[in,out] dev       DFPlayer mini device
 *
 * @return  number of tracks in @p folder
 * @return  DFPLAYER_ERR_TIMEOUT on timeout waiting for reply
 */
int dfplayer_count_files(dfplayer_t *dev);

/**
 * @brief   Count the number of files in the given folder
 *
 * @param[in,out] dev       DFPlayer mini device
 * @param[in]     folder    folder to play from [1-99]
 *
 * @return  number of tracks in @p folder
 * @return  DFPLAYER_ERR_TIMEOUT on timeout waiting for reply
 */
int dfplayer_count_files_in_folder(dfplayer_t *dev, unsigned folder);

#ifdef __cplusplus
}
#endif

#endif /* DFPLAYER_H */
/** @} */
