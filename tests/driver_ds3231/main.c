/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for the DS1307 RTC driver
 *
 * @author      Martine Lenders <m.lenders@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "shell.h"
#include "xtimer.h"
#include "ds3231.h"
#include "ds3231_params.h"

#define ISOSTR_LEN      (20U)

static ds3231_t _dev;

/* 2010-09-22T15:10:42 is the author date of RIOT's initial commit ;-) */
static struct tm _riot_bday = {
    .tm_sec = 42,
    .tm_min = 10,
    .tm_hour = 15,
    .tm_wday = 3,
    .tm_mday = 22,
    .tm_mon = 8,
    .tm_year = 110
};


/* parse ISO date string (YYYY-MM-DDTHH:mm:ss) to struct tm */
static int _tm_from_str(const char *str, struct tm *time)
{
    char tmp[5];

    if (strlen(str) != ISOSTR_LEN - 1) {
        return -1;
    }
    if ((str[4] != '-') || (str[7] != '-') || (str[10] != 'T') ||
        (str[13] != ':') || (str[16] != ':')) {
        return -1;
    }

    memset(time, 0, sizeof(struct tm));

    memcpy(tmp, str, 4);
    tmp[4] = '\0';
    str += 5;
    time->tm_year = atoi(tmp);

    memcpy(tmp, str, 2);
    tmp[3] = '\0';
    str += 3;
    time->tm_mon = atoi(tmp);

    memcpy(tmp, str, 2);
    str += 3;
    time->tm_mday = atoi(tmp);

    memcpy(tmp, str, 2);
    str += 3;
    time->tm_hour = atoi(tmp);

    memcpy(tmp, str, 2);
    str += 3;
    time->tm_min = atoi(tmp);

    memcpy(tmp, str, 2);
    time->tm_sec = atoi(tmp);

    return 0;
}

static int _cmd_get(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    char dstr[ISOSTR_LEN];

    struct tm time;
    ds3231_get_time(&_dev, &time);

    size_t pos = strftime(dstr, ISOSTR_LEN, "%Y-%m-%dT%H:%M:%S", &time);
    dstr[pos] = '\0';
    printf("The current time is: %s\n", dstr);

    return 0;
}

static int _cmd_set(int argc, char **argv)
{
    if (argc != 2) {
        printf("usage: %s <iso-date-str YYYY-MM-DDTHH:mm:ss>\n", argv[0]);
        return 1;
    }

    if (strlen(argv[1]) != (ISOSTR_LEN - 1)) {
        puts("error: input date string has invalid length");
        return 1;
    }

    struct tm target_time;
    int res = _tm_from_str(argv[1], &target_time);
    if (res != 0) {
        puts("error: unable do parse input date string");
        return 1;
    }

    ds3231_set_time(&_dev, &target_time);

    printf("success: time set to %s\n", argv[1]);
    return 0;
}

static int _cmd_power(int argc, char **argv)
{
    int res;

    if (argc != 2) {
        printf("usage: %s <'0' or '1'>\n", argv[0]);
        return 1;
    }

    if (argv[1][0] == '0') {
        res = ds3231_poweroff(&_dev);
        if (res == 0) {
            puts("success: device powered down");
        }
        else {
            puts("error: unable to power down device");
        }
    }
    else if (argv[1][0] == '1') {
        res = ds3231_poweron(&_dev);
        if (res == 0) {
            puts("success: device powered up");
        }
        else {
            puts("error: unable to power up device");
        }
    }
    else {
        puts("error: unable to parse command");
        return 1;
    }

    return 0;
}

static int _cmd_test(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int res;
    struct tm time;

    puts("testing device now");

    /* disable RTC */
    res = ds3231_poweroff(&_dev);
    if (res != 0) {
        puts("error: unable to power off device");
        return 1;
    }

    /* set time to RIOT birthdate */
    res = ds3231_set_time(&_dev, &_riot_bday);
    if (res != 0) {
        puts("error: unable to set time");
        return 1;
    }

    /* read time and compare to initial value */
    res = ds3231_get_time(&_dev, &time);
    if (res != 0) {
        puts("error: unable to read time");
        return 1;
    }

    if (mktime(&_riot_bday) != mktime(&time)) {
        puts("error: device time has unexpected value");
        return 1;
    }

    /* power up RTC and wait a short while. Then check if time progressed */
    res = ds3231_poweron(&_dev);
    if (res != 0) {
        puts("error: unable to power up device");
        return 1;
    }
    xtimer_sleep(2);
    res = ds3231_get_time(&_dev, &time);
    if (res != 0) {
        puts("error: unable to read time");
        return 1;
    }

    if (!(mktime(&time) > mktime(&_riot_bday))) {
        puts("error: time did not progress since power on");
        return 1;
    }

    puts("OK");
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "time_get", "init as output (push-pull mode)", _cmd_get },
    { "time_set", "init as input w/o pull resistor", _cmd_set },
    { "power", "power device on or off", _cmd_power },
    { "test", "test if the device is working properly", _cmd_test},
    { NULL, NULL, NULL }
};

int main(void)
{
    int res;

    puts("DS3231 RTC test\n");

    /* initialize the device */
    res = ds3231_init(&_dev, &ds3231_params[0]);
    if (res != 0) {
        puts("error: unable to initialize DS3231 [I2C initialization error]");
        return 1;
    }

    /* start the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
