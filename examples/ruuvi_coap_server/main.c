/*
 * Copyright (c) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       IoThon hackathon demo application to showcase CoAP on the ruuvitag
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "msg.h"
#include "fmt.h"
#include "shell.h"
#include "saul_reg.h"

#include "net/gcoap.h"

#define MAIN_QUEUE_SIZE (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static saul_reg_t *sense[4];
static uint8_t stype[4] = {
    SAUL_SENSE_TEMP,
    SAUL_SENSE_HUM,
    SAUL_SENSE_PRESS,
    SAUL_SENSE_ACCEL,
};

static const char *hello = "Hello RuuviTag, hello IoThon!";

static void get_env_reading(char *buf, saul_reg_t *sensor)
{
    size_t pos;
    phydat_t d;
    saul_reg_read(sensor, &d);
    if (d.scale < 0) {
        pos = fmt_s16_dfp(buf, d.val[0], (unsigned)(d.scale * -1));
    } else {
        pos = fmt_u32_dec(buf, (((uint32_t)d.val[0])));
    }
    sprintf(&buf[pos], "%s", phydat_unit_to_str(d.unit));
}

static ssize_t handle_env(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    printf("coap: handle env\n");

    /* acquire sensor readings and format them into buffer */
    char tmp[3][15];
    for (int i = 0; i < 3; i++) {
        get_env_reading(tmp[i], sense[i]);
    }

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    char json[128];
    sprintf(json, "{\n\t\"temp\":\"%s\",\n\t\"hum\":\"%s\",\n\t\"pres\":\"%s\"\n}",
            tmp[0], tmp[1], tmp[2]);
    memcpy(pdu->payload, json, strlen(json));
    printf("json: %s\n", json);
    return gcoap_finish(pdu, strlen(json), COAP_FORMAT_TEXT);
}

static ssize_t handle_acc(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    printf("caop: handle acc\n");

    phydat_t d;
    char res[3][15];
    saul_reg_read(sense[3], &d);
    for (int i = 0; i < 3; i++) {
        size_t pos = fmt_s16_dfp(res[i], d.val[i], 3);
        res[i][pos++] = 'g';
        res[i][pos] = '\0';
    }

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    char foo[128];
    sprintf(foo, "{\n\t\"x\":\"%s\",\n\t\"y\":\"%s\",\n\t\"z\":\"%s\"\n}",
            res[0], res[1], res[2]);
    memcpy(pdu->payload, foo, strlen(foo));
    printf("acc: %s\n", foo);
    return gcoap_finish(pdu, strlen(foo), COAP_FORMAT_TEXT);
}

static ssize_t handle_hello(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    printf("coap: handle hello\n");

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memcpy(pdu->payload, hello, strlen(hello));
    return gcoap_finish(pdu, strlen(hello), COAP_FORMAT_TEXT);
}

/* CoAP resources */
static const coap_resource_t resources[] = {
    { "/ruuvi/acc", COAP_GET, handle_acc },
    { "/ruuvi/env", COAP_GET, handle_env },
    { "/ruuvi/hello", COAP_GET, handle_hello },
};

static gcoap_listener_t listener = {
    (coap_resource_t *)&resources[0],
    sizeof(resources) / sizeof(resources[0]),
    NULL,
};


int main(void)
{
    puts("IoThon CoAP server @ ruuvitag example\n");

    /* for the thread running the shell */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    /* get SAUL handles for sensors */
    for (int i = 0; i < 4; i++) {
        sense[i] = saul_reg_find_type(stype[i]);
        if (sense[i] == NULL) {
            printf("error: unable to find sensor %i\n", i);
            return 1;
        }
    }

    /* register coap listener */
    gcoap_register_listener(&listener);

    /* start shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should never be reached */
    return 0;
}
