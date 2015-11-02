/*
 * Copyright (C) 2013 INRIA
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief Thread test application
 *
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include "thread.h"
#include "mm.h"
#include "msg.h"

char t2_stack[THREAD_STACKSIZE_MAIN];

static msg_t msg, msg2;

void *second_thread(void *arg)
{
    (void) arg;

    MM2H;
    MM2L;
    MM2H;
    MM2L;

    msg_receive(&msg2);

    return NULL;
}

int main(void)
{
    MM_INIT;

    MM1H;
    MM2H;
    MM2L;
    MM1L;

    (void) thread_create(
            t2_stack, sizeof(t2_stack),
            THREAD_PRIORITY_MAIN - 1, CREATE_WOUT_YIELD | CREATE_STACKTEST,
            second_thread, NULL, "nr2");

    MM1H;
    MM1L;
    msg_receive(&msg);
    thread_yield();
    MM1H;
    MM1L;



    return 0;
}
