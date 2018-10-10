
#include <string.h>

#include "assert.h"
#include "xtimer.h"
#include "byteorder.h"
#include "dfplayer.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

#define STARTUP_RETRY_CNT   (3U)
#define STARTUP_RETRY_DELAY (250U * US_PER_MS)

#define POS_START           (0U)
#define POS_VER             (1U)
#define POS_LEN             (2U)
#define POS_CMD             (3U)
#define POS_FEEDBACK        (4U)
#define POS_PARAM           (5U)
#define POS_CSUM            (7U)
#define POS_END             (9U)

#define START_BYTE          (0x7e)
#define END_BYTE            (0xef)
#define VERSION             (0xff)
#define LEN                 (0x06)

#define CMD_NEXT            (0x01)
#define CMD_PREV            (0x02)
#define CMD_PLAY_TRACK      (0x03)
#define CMD_VOL_INC         (0x04)
#define CMD_VOL_DEC         (0x05)
#define CMD_VOL_SET         (0x06)
#define CMD_EQ              (0x07)
#define CMD_LOOP            (0x08)  /* differs from documentation... */
#define CMD_SOURCE_SELECT   (0x09)
#define CMD_STANDBY         (0x0a)
#define CMD_WAKEUP          (0x0b)
#define CMD_RESET           (0x0c)
#define CMD_PLAYBACK        (0x0d)
#define CMD_PAUSE           (0x0e)
#define CMD_PLAY_FOLDER     (0x0f)
#define CMD_VOL             (0x10)
#define CMD_REPEAT_PLAY     (0x11)
#define CMD_PLAY_RANDOM     (0x18)  /* undocumented feature */

#define CMD_FINISH_U        (0x3c)
#define CMD_FINISH_TF       (0x3d)
#define CMD_FINISH_FL       (0x3e)
#define CMD_INIT_PARAMS     (0x3f)
#define CMD_RETRANSMIT      (0x40)
#define CMD_REPLY           (0x41)
#define CMD_QUERY_STATUS    (0x42)
#define CMD_QUERY_VOL       (0x43)
#define CMD_QUERY_EQ        (0x44)
#define CMD_QUERY_MODE      (0x45)
#define CMD_QUERY_VER       (0x46)
#define CMD_NUM_FILES_TF    (0x47)
#define CMD_NUM_FILES_U     (0x48)
#define CMD_NUM_FILES_FL    (0x49)
#define CMD_KEEP_ON         (0x4a)
#define CMD_CUR_TRACK_TF    (0x4b)
#define CMD_CUR_TRACK_U     (0x4c)
#define CMD_CUR_TRACK_FL    (0x4d)
#define CMD_COUNT_FOLDER    (0x4e)  /* undocumented feature */

#define NO_REPLY            (0xff)

#define SRC_UDISK           (0x01)
#define SRC_TF              (0x02)
#define SRC_PC              (0x04)
#define SRC_FLASH           (0x08)

#define FLAG_RESP           (1u << 7)       /* chosen arbitrarily */
#define FLAG_RETRANSMIT     (1u << 8) /* TODO */
#define FLAG_MASK           (THREAD_FLAG_TIMEOUT | FLAG_RESP)

#define EVENT_USED          (0x80)

/* TODO: remove and use global context */
static event_queue_t _q;
static char _stack[DFPLAYER_STACKSIZE];

static uint16_t _csum(uint8_t *buf)
{
    uint16_t res = 0;
    for (unsigned pos = POS_VER; pos < POS_CSUM; pos++) {
        res += buf[pos];
    }
    return -res;
}

static int _cmd(dfplayer_t *dev, uint8_t cmd, uint16_t param, uint8_t resp_code)
{
    assert(dev);
    int ret = DFPLAYER_OK;

    mutex_lock(&dev->lock);

    /* set expected result */
    dev->exp_code = resp_code;

    /* build command packet */
    uint8_t buf[DFPLAYER_PKTLEN] = { START_BYTE, VERSION, LEN, cmd, 0,
                                     0, 0, 0, 0, END_BYTE };
    byteorder_htobebufs(buf + POS_PARAM, param);
    byteorder_htobebufs(buf + POS_CSUM, _csum(buf));
    uart_write(dev->uart, buf, DFPLAYER_PKTLEN);
    if (resp_code != NO_REPLY) {
        dev->waiter = (thread_t *)sched_active_thread;
        xtimer_set_timeout_flag(&dev->to_timer, DFPLAYER_TIMEOUT);
        thread_flags_t f = thread_flags_wait_any(FLAG_MASK);
        xtimer_remove(&dev->to_timer);
        if (f & FLAG_RESP) {
            ret = (int)dev->rx_data;
        }
        else {
            ret = DFPLAYER_ERR_TIMEOUT;
        }
        dev->exp_code = NO_REPLY;
    }
    mutex_unlock(&dev->lock);
    return ret;
}

static void _on_rx_byte(void *arg, uint8_t data)
{
    dfplayer_t *dev = (dfplayer_t *)arg;

    if ((dev->rx_pos == POS_START) && (data == START_BYTE)) {
        ++dev->rx_pos;
        return;
    }
    else if (dev->rx_pos == POS_END) {
        if (data == END_BYTE) {
            /* validate csum */
            uint16_t csum = byteorder_bebuftohs(&dev->rx_buf[POS_CSUM]);
            if (csum == _csum(dev->rx_buf)) {
                uint8_t code = dev->rx_buf[POS_CMD];
                uint16_t param = byteorder_bebuftohs(&dev->rx_buf[POS_PARAM]);

                if (code == dev->exp_code) {
                    dev->rx_data = param;
                    thread_flags_set(dev->waiter, FLAG_RESP);
                }
                // else if (code == CMD_FINISH_TF) {
                    dev->async_event.flags = EVENT_USED;
                    dev->async_event.code = code;
                    dev->async_event.param = param;
                    event_post(&_q, &dev->async_event.super);
                // }
            }
            else {
                DEBUG("CSUM wrong\n");
            }
        }
    }
    else {
        dev->rx_buf[dev->rx_pos++] = data;
        return;
    }

    /* whenever we end up here we reset the RX buffer */
    dev->rx_pos = 0;
}

static void _on_pkt(event_t *arg)
{
    dfplayer_event_t *e = (dfplayer_event_t *)arg;

    DEBUG("[dfplayer] _on_pkt: CMD 0x%02x, param: %u\n",
          (int)e->code, (unsigned)e->param);
}

/* TODO: remove and use global context */
static void *_thread(void *arg)
{
    (void)arg;
    event_queue_init(&_q);
    event_loop(&_q);
    return NULL;
}

int dfplayer_init(dfplayer_t *dev, const dfplayer_params_t *params)
{
    assert(dev);
    assert(params);

    mutex_init(&dev->lock);

    dev->uart = params->uart;
    dev->rx_pos = 0;
    dev->exp_code = NO_REPLY;
    dev->async_event.super.handler = _on_pkt;
    dev->async_event.dev = dev;

    /* run event queue thread */
    /* TODO: move towards system wide solution... */
    thread_create(_stack, sizeof(_stack), DFPLAYER_PRIO, 0,
                  _thread, NULL, "dfplayer");

    /* initialize UART */
    if (uart_init(dev->uart, params->baudrate, _on_rx_byte, dev) != UART_OK) {
        return DFPLAYER_ERR_UART;
    }

    /* read the source configuration from the device */
    for (unsigned retries = 0; retries < STARTUP_RETRY_CNT; retries++) {
        int res = _cmd(dev, CMD_INIT_PARAMS, 0, CMD_INIT_PARAMS);
        if (res == SRC_TF) {
            return DFPLAYER_OK;
        }
    }

    return DFPLAYER_ERR_TIMEOUT;
}

int dfplayer_reset(dfplayer_t *dev)
{
    return _cmd(dev, CMD_RESET, 0, CMD_INIT_PARAMS);
}

void dfplayer_standby(dfplayer_t *dev)
{
    _cmd(dev, CMD_STANDBY, 0, NO_REPLY);
}

void dfplayer_wakeup(dfplayer_t *dev)
{
    _cmd(dev, CMD_WAKEUP, 0, NO_REPLY);
}

int dfplayer_ver(dfplayer_t *dev)
{
    return _cmd(dev, CMD_QUERY_VER, 0, CMD_QUERY_VER);
}

int dfplayer_status(dfplayer_t *dev)
{
    return _cmd(dev, CMD_QUERY_STATUS, 0, CMD_QUERY_STATUS);
}

void dfplayer_vol_set(dfplayer_t *dev, unsigned level)
{
    if (level > DFPLAYER_VOL_MAX) {
        level = DFPLAYER_VOL_MAX;
    }
    _cmd(dev, CMD_VOL_SET, (uint16_t)level, NO_REPLY);
}

void dfplayer_vol_up(dfplayer_t *dev)
{
    _cmd(dev, CMD_VOL_INC, 0, NO_REPLY);
}

void dfplayer_vol_down(dfplayer_t *dev)
{
    _cmd(dev, CMD_VOL_DEC, 0, NO_REPLY);
}

int dfplayer_vol_get(dfplayer_t *dev)
{
    return _cmd(dev, CMD_QUERY_VOL, 0, CMD_QUERY_VOL);
}

void dfplayer_eq_set(dfplayer_t *dev, dfplayer_eq_t eq)
{
    assert(eq <= DFPLAYER_BASE);
    _cmd(dev, CMD_EQ, (uint16_t)eq, NO_REPLY);
}

int dfplayer_eq_get(dfplayer_t *dev)
{
    return _cmd(dev, CMD_QUERY_EQ, 0, CMD_QUERY_EQ);
}

int dfplayer_mode_get(dfplayer_t *dev)
{
    return _cmd(dev, CMD_QUERY_MODE, 0, CMD_QUERY_MODE);
}

void dfplayer_play(dfplayer_t *dev)
{
    _cmd(dev, CMD_PLAYBACK, 0, NO_REPLY);
}

void dfplayer_play_track(dfplayer_t *dev, unsigned track)
{
    _cmd(dev, CMD_PLAY_TRACK, (uint16_t)track, NO_REPLY);
}

void dfplayer_play_random(dfplayer_t *dev)
{
    _cmd(dev, CMD_PLAY_RANDOM, 0, NO_REPLY);
}

void dfplayer_play_folder(dfplayer_t *dev, unsigned folder, unsigned track)
{
    uint16_t p = (((uint8_t)folder << 8) | (uint8_t)track);
    _cmd(dev, CMD_PLAY_FOLDER, p, NO_REPLY);
}

void dfplayer_loop_track(dfplayer_t *dev, unsigned track)
{
    _cmd(dev, CMD_LOOP, (uint16_t)track, NO_REPLY);
}

void dfplayer_pause(dfplayer_t *dev)
{
    _cmd(dev, CMD_PAUSE, 0, NO_REPLY);
}

void dfplayer_next(dfplayer_t *dev)
{
    _cmd(dev, CMD_NEXT, 0, NO_REPLY);
}

void dfplayer_prev(dfplayer_t *dev)
{
    _cmd(dev, CMD_PREV, 0, NO_REPLY);
}

int dfplayer_current_track(dfplayer_t *dev)
{
    return _cmd(dev, CMD_CUR_TRACK_TF, 0, CMD_CUR_TRACK_TF);
}

int dfplayer_count_files(dfplayer_t *dev)
{
    return _cmd(dev, CMD_NUM_FILES_U, 0, CMD_NUM_FILES_U);
}

int dfplayer_count_files_in_folder(dfplayer_t *dev, unsigned folder)
{
    return _cmd(dev, CMD_COUNT_FOLDER, (uint16_t)folder, CMD_COUNT_FOLDER);
}
