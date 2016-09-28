/*
 * Copyright (C) 2014 Freie Universität Berlin
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
 * @brief       Hello World application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>

#include "dev.h"

#define DEV     __attribute__((used,section(".devs")))

extern uint32_t _sdevs;
extern uint32_t _edevs;

typedef struct {
    char *name;
} devdesc_t;

typedef struct {
    const char *name;
} devparam_t;

typedef struct {
    const devdesc_t *dev;
    const devparam_t *params;
} devinit_t;


static devdesc_t deva;
static devdesc_t devb;

const devparam_t deva_param = {
    .name = "foomann"
};
const devparam_t devb_param = {
    .name = "Mister pister"
};

DEV devinit_t deva_ent = {
    &deva,
    &deva_param
};
DEV devinit_t devb_ent = {
    &devb,
    &devb_param
};


DEV devinit_t devc_ent = {
    &devb,
    { .name = "kukuck" }
};



int main(void)
{
    puts("Section test");

    printf("sdevs: %p, edevs: %p, size %i\n", (void *)&_sdevs, (void *)&_edevs, (int)((&_edevs - &_sdevs) * 4));


    printf("addr of deva is %p\n", (void *)&deva);
    printf("addr of deva_param is %p\n", (void *)&deva_param);
    printf("addr of deva_ent is %p\n", (void *)&deva_ent);

    devinit_t *tmp = (devinit_t *)(&_sdevs);
    while (tmp < (devinit_t *)&_edevs) {
        printf("Dev is %s\n", tmp->params->name);
        ++tmp;
    }


    return 0;
}
