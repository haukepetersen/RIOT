/*
 * Copyright (C) 2008, 2009, 2010  Kaspar Schleiser <kaspar@schleiser.de>
 * Copyright (C) 2013 INRIA
 * Copyright (C) 2013 Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
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
 * @brief       Default application that shows a lot of functionality of RIOT
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "devreg.h"
#include "shell.h"
#include "shell_commands.h"


static int cmd_list(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int i = 0;
    device_t *dev = devreg_getnext(NULL);
    while (dev) {
        printf("dev %2i: fam %02x, group %02x, model %04x", i++,
               (int)device_get_fam(dev), (int)device_get_group(dev),
               (int)device_get_model(dev));
#ifdef DEVELHELP
        printf(" name: '%s'", dev->name);
#endif
        puts("");
    }

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "list", "list all available devices", cmd_list },
    { NULL, NULL, NULL }
};

int main(void)
{

    (void) puts("Welcome to RIOT!");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
