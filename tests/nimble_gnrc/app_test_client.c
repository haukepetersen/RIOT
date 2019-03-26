
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "shell.h"
#include "net/sock/udp.h"
#include "net/ipv6/addr.h"

#include "app_test_client.h"
#include "nimble_gnrc_test_conf.h"

#define FLAG_SYNC       (1u << 1)

static volatile uint32_t _last_rx_seq;

static uint32_t _txbuf[UDP_BUFSIZE / 4];
static ipv6_addr_t _server_addr;

static void _send(uint32_t type, uint32_t seq, size_t len)
{
    int res = 0;
    (void)res;

    assert(len >= 8);
    assert(len <= UDP_BUFSIZE);
    printf("# Sending: size %5u seq %5u\n", len, (unsigned)seq);

    _txbuf[POS_TYPE] = type;
    _txbuf[POS_SEQ] = seq;
    // app_udp_send(&_server_addr, _txbuf, len);
}

static int _cmd_inctest(int argc, char **argv)
{
    size_t step = 10;
    size_t limit = UDP_BUFSIZE;

    if ((argc == 2) && strncmp(argv[1], "help", 4) == 0) {
        printf("usage: %s [step [limit]]\n", argv[0]);
        return 0;
    }

    /* parse params */
    if (argc >= 2) {
        step = (size_t)atoi(argv[1]);
    }
    if (argc >= 3) {
        limit = (size_t)atoi(argv[2]);
        if ((limit < 8) || (limit > UDP_BUFSIZE)) {
            puts("err: invalid limit payload length given");
            return 1;
        }
    }

    /* send out packets with increasing payload size */
    thread_flags_clear(FLAG_SYNC);
    uint32_t seq = 0;
    uint32_t time = xtimer_now_usec();
    for (size_t i = 8; i < limit; i += step) {
        _send(TYPE_INCTEST, ++seq, i);
        thread_flags_wait_all(FLAG_SYNC);
        if (_last_rx_seq != seq) {
            printf("# err: bad sequence number in response (%u)\n",
                   (unsigned)_last_rx_seq);
            assert(0);
        }
    }
    time = (xtimer_now_usec() - time);
    puts("# TEST COMPLETE");
    printf("-> runtime: %ums\n", (unsigned)(time / 1000));
    return 0;
}

static int _cmd_floodtest(int argc, char **argv)
{
    size_t pktsize = PKTSIZE_DEFAULT;
    unsigned limit = FLOOD_DEFAULT;

    if ((argc == 2) && strncmp(argv[1], "help", 4) == 0) {
        printf("usage: %s [payload_size [num_of_packets]]\n", argv[0]);
        return 0;
    }

    if (argc >= 2) {
        pktsize = (size_t)atoi(argv[1]);
        if ((pktsize < 8) || (pktsize > UDP_BUFSIZE)) {
            puts("err: invalid packet size given");
            return 1;
        }
    }
    if (argc >= 3) {
        limit = (unsigned)atoi(argv[2]);
    }

    /* now lets flood */
    uint32_t seq = 0;
    uint32_t time = xtimer_now_usec();
    unsigned sum = 0;
    for (unsigned i = 0; i < limit; i++) {
        _send(TYPE_FLOODING, ++seq, pktsize);
        sum += pktsize;
    }
    /* we wait for the last packet to be returned to compensate any internal
     * packet buffering, but we also include this extra packet transmission in
     * the throughput calculation */
    sum += pktsize;
    while (_last_rx_seq != seq) {}
    time = (xtimer_now_usec() - time);
    puts("# TEST COMPLETE");
    printf("-> runtime: %ums\n", (unsigned)(time / 1000));
    printf("-> ~ %u bytes/s\n", (unsigned)((sum * 1000) / (time / 1000)));

    return 0;
}

static const shell_command_t _cmds[] = {
    { "inctest", "send stream of payloads of incremental size", _cmd_inctest },
    { "flood", "flood connection with data", _cmd_floodtest },
    { NULL, NULL, NULL }
};

void app_test_client_run(void)
{
    /* start shell */
    puts("# Shell is now available");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(_cmds, line_buf, SHELL_DEFAULT_BUFSIZE);
}
