
#include <string.h>

#include "assert.h"
#include "xtimer.h"
#include "byteorder.h"
#include "dfplayer.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

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
#define CMD_TRACK           (0x03)
#define CMD_VOL_INC         (0x04)
#define CMD_VOL_DEC         (0x05)
#define CMD_VOL_SET         (0x06)
#define CMD_EQ              (0x07)
#define CMD_MODE            (0x08)
#define CMD_SOURCE_SELECT   (0x09)
#define CMD_STANDBY         (0x0a)
#define CMD_WAKEUP          (0x0b)
#define CMD_RESET           (0x0c)
#define CMD_PLAYBACK        (0x0d)
#define CMD_PAUSE           (0x0e)
#define CMD_PLAY_F          (0x0f)
#define CMD_VOL             (0x10)
#define CMD_REPEAT_PLAY     (0x11)

#define CMD_RETRANSMIT      (0x40)
#define CMD_REPLY           (0x41)

#define CMD_INVALID         (0xff)

#define LAST_BYTE           (9U)

#define FLAG_RESP           (1u << 7)       /* chosen arbitrarily */
#define FLAG_RETRANSMIT     (1u << 8) /* TODO */
#define FLAG_MASK           (THREAD_FLAG_TIMEOUT | FLAG_RESP)

#define EVENT_USED          (0x80)

/* TODO: remove and use global context */
static event_queue_t _q;
static char _stack[DFPLAYER_STACKSIZE];

/* TODO remove */
#if ENABLE_DEBUG
static void _dump(uint8_t *pkt) {
    DEBUG("  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
          pkt[0], pkt[1], pkt[2], pkt[3], pkt[4], pkt[5], pkt[6], pkt[7],
          pkt[8], pkt[9]);
}
#else
static void _dump(uint8_t *pkt)
{
    (void)pkt;
}
#endif

static uint16_t _csum(uint8_t *buf)
{
    uint16_t res = 0;
    for (unsigned pos = POS_VER; pos < POS_CSUM; pos++) {
        res += buf[pos];
    }
    return -res;
}

static int _cmd(dfplayer_t *dev, uint8_t cmd, uint16_t param,
                uint8_t resp_code, uint16_t *resp_param)
{
    int ret = DFPLAYER_OK;

    mutex_lock(&dev->lock);

    /* set expected result */
    dev->exp_code = resp_code;

    /* build command packet */
    uint8_t buf[DFPLAYER_PKTLEN] = { START_BYTE, VERSION, LEN, cmd, 1,
                                     0, 0, 0, 0, END_BYTE };
    byteorder_htobebufs(buf + POS_PARAM, param);
    byteorder_htobebufs(buf + POS_CSUM, _csum(buf));
    DEBUG("_cmd:"); _dump(buf);
    uart_write(dev->uart, buf, DFPLAYER_PKTLEN);
    if (resp_code != CMD_INVALID) {
        dev->waiter = (thread_t *)sched_active_thread;
        xtimer_set_timeout_flag(&dev->to_timer, DFPLAYER_TIMEOUT);
        thread_flags_t f = thread_flags_wait_any(FLAG_MASK);
        xtimer_remove(&dev->to_timer);
        if (f & FLAG_RESP) {
            if (resp_param) {
                *resp_param = dev->rx_data;
            }
        }
        else {
            ret = DFPLAYER_TIMEOUT;
        }
        dev->exp_code = CMD_INVALID;
        DEBUG("got reply");
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
            uint16_t csum = byteorder_bebuftohs(dev->rx_buf.pkt.csum);
            if (csum == _csum(dev->rx_buf.raw)) {
                uint8_t code = dev->rx_buf.pkt.cmd;
                uint16_t param = byteorder_bebuftohs(dev->rx_buf.pkt.param);

                if (code == dev->exp_code) {
                    dev->rx_data = param;
                    thread_flags_set(dev->waiter, FLAG_RESP);
                }
                /*
                switch (code) {
                    FLAG_RESP
                }
                */
                else {
                    dev->async_event.flags = EVENT_USED;
                    dev->async_event.code = code;
                    dev->async_event.param = param;
                    event_post(&_q, &dev->async_event.super);
                }
            }
            else {
                DEBUG("CSUM wrong\n");
            }
        }
    }
    else {
        dev->rx_buf.raw[dev->rx_pos++] = data;
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
    dev->exp_code = CMD_INVALID;
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

    DEBUG("init ok\n");

    return DFPLAYER_OK;
}

void dfplayer_wakeup(dfplayer_t *dev)
{
    assert(dev);
    _cmd(dev, CMD_WAKEUP, 0, CMD_INVALID, NULL);
}

void dfplayer_vol_up(dfplayer_t *dev)
{
    assert(dev);
    _cmd(dev, CMD_VOL_INC, 0, CMD_REPLY, NULL);
}

void dfplayer_vol_down(dfplayer_t *dev)
{
    assert(dev);
    _cmd(dev, CMD_VOL_DEC, 0, CMD_REPLY, NULL);
}

void dfplayer_vol_set(dfplayer_t *dev, uint16_t level)
{
    assert(dev);
    assert(level <= DFPLAYER_VOL_MAX);
    _cmd(dev, CMD_VOL_SET, level, CMD_INVALID, NULL);
}

void dfplayer_play(dfplayer_t *dev)
{
    assert(dev);
    _cmd(dev, CMD_PLAYBACK, 0, CMD_INVALID, NULL);
}

void dfplayer_pause(dfplayer_t *dev)
{
    assert(dev);
    _cmd(dev, CMD_PAUSE, 0, CMD_INVALID, NULL);
}

void dfplayer_next(dfplayer_t *dev)
{
    assert(dev);
    _cmd(dev, CMD_NEXT, 0, CMD_INVALID, NULL);
}

void dfplayer_prev(dfplayer_t *dev)
{
    assert(dev);
    _cmd(dev, CMD_PREV, 0, CMD_INVALID, NULL);
}
