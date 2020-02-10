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

typedef enum {
    A,
    B,
} foo_t;

void a(foo_t f)
{
    printf("%i\n", f);
}

int main(void)
{
    a(A);
    a(B);
    a(23);

    foo_t foo = 23;
    a(foo);

    return 0;
}
