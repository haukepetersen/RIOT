/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_ggs_riot
 * @{
 *
 * @file
 * @brief       RIOT specific LED GATT service for Gorm
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include "board.h"
#include "net/ggs/riot.h"
#include "net/gorm/gatt/desc.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

/* we assume them all initially turned off */
static uint8_t _led_state = 0;

#if defined(LED7_ON)
#define NUMOF_LED               (8U)
#elif defined(LED6_ON)
#define NUMOF_LED               (7U)
#elif defined(LED5_ON)
#define NUMOF_LED               (6U)
#elif defined(LED4_ON)
#define NUMOF_LED               (5U)
#elif defined(LED3_ON)
#define NUMOF_LED               (4U)
#elif defined(LED2_ON)
#define NUMOF_LED               (3U)
#elif defined(LED1_ON)
#define NUMOF_LED               (2U)
#elif defined(LED0_ON)
#define NUMOF_LED               (1U)
#else
#error "Gorm GATT service 'LED': unable to build - no LEDs available"
#endif

static inline void _write(unsigned led, uint8_t state)
{
    if (state) {
        _led_state |= (1 << led);
        switch (led) {
            case 0: LED0_ON;    break;
#ifdef LED1_ON
            case 1: LED1_ON;    break;
#endif
#ifdef LED2_ON
            case 2: LED2_ON;    break;
#endif
#ifdef LED3_ON
            case 3: LED3_ON;    break;
#endif
#ifdef LED4_ON
            case 4: LED4_ON;    break;
#endif
#ifdef LED5_ON
            case 5: LED5_ON;    break;
#endif
#ifdef LED6_ON
            case 6: LED6_ON;    break;
#endif
#ifdef LED7_ON
            case 7: LED7_ON;    break;
#endif
            default:            break;  /* do nothing */
        }
    }
    else {
        _led_state &= ~(1 << led);
        switch (led) {
            case 0: LED0_OFF;   break;
#ifdef LED1_ON
            case 1: LED1_OFF;   break;
#endif
#ifdef LED2_ON
            case 2: LED2_OFF;   break;
#endif
#ifdef LED3_ON
            case 3: LED3_OFF;   break;
#endif
#ifdef LED4_ON
            case 4: LED4_OFF;   break;
#endif
#ifdef LED5_ON
            case 5: LED5_OFF;   break;
#endif
#ifdef LED6_ON
            case 6: LED6_OFF;   break;
#endif
#ifdef LED7_ON
            case 7: LED7_OFF;   break;
#endif
            default:            break;  /* do nothing */
        }
    }
}

static size_t _led_cb(const gorm_gatt_char_t *c, uint8_t method,
                      uint8_t *buf, size_t buf_len)
{
    if (buf_len < 1) {
        return 0;
    }

    unsigned led = (unsigned)c->arg;


    switch (method) {
        case GORM_GATT_READ:
            buf[0] = ((_led_state >> led) & 0x1);
            return 1;
        case GORM_GATT_WRITE:
            _write(led, buf[0]);
            return 1;
        default:
            return 0;
    }
}

static const gorm_gatt_desc_t _led_desc[] = {
    GORM_GATT_DESC_FMT(BLE_UNIT_BLE_FMT_BOOL, 0, BLE_UNIT_NONE),
};

static const gorm_gatt_char_t _led_chars[] = {
    {
        .cb       = _led_cb,
        .arg      = (void *)0,
        .type     = GORM_UUID(GGS_RIOT_UUID_LED_CHAR, &ggs_uuid_riot_base),
        .perm     = (BLE_ATT_READ | BLE_ATT_WRITE),
        .desc_cnt = 1,
        .desc     = _led_desc,
    },
#ifdef LED1_ON
    {
        .cb       = _led_cb,
        .arg      = (void *)1,
        .type     = GORM_UUID(GGS_RIOT_UUID_LED_CHAR, &ggs_uuid_riot_base),
        .perm     = (BLE_ATT_READ | BLE_ATT_WRITE),
        .desc_cnt = 1,
        .desc     = _led_desc,
    },
#endif
#ifdef LED2_ON
    {
        .cb       = _led_cb,
        .arg      = (void *)2,
        .type     = GORM_UUID(GGS_RIOT_UUID_LED_CHAR, &ggs_uuid_riot_base),
        .perm     = (BLE_ATT_READ | BLE_ATT_WRITE),
        .desc_cnt = 1,
        .desc     = _led_desc,
    },
#endif
#ifdef LED3_ON
    {
        .cb       = _led_cb,
        .arg      = (void *)3,
        .type     = GORM_UUID(GGS_RIOT_UUID_LED_CHAR, &ggs_uuid_riot_base),
        .perm     = (BLE_ATT_READ | BLE_ATT_WRITE),
        .desc_cnt = 1,
        .desc     = _led_desc,
    },
#endif
#ifdef LED4_ON
    {
        .cb       = _led_cb,
        .arg      = (void *)4,
        .type     = GORM_UUID(GGS_RIOT_UUID_LED_CHAR, &ggs_uuid_riot_base),
        .perm     = (BLE_ATT_READ | BLE_ATT_WRITE),
        .desc_cnt = 1,
        .desc     = _led_desc,
    },
#endif
#ifdef LED5_ON
    {
        .cb       = _led_cb,
        .arg      = (void *)5,
        .type     = GORM_UUID(GGS_RIOT_UUID_LED_CHAR, &ggs_uuid_riot_base),
        .perm     = (BLE_ATT_READ | BLE_ATT_WRITE),
        .desc_cnt = 1,
        .desc     = _led_desc,
    },
#endif
#ifdef LED6_ON
    {
        .cb       = _led_cb,
        .arg      = (void *)6,
        .type     = GORM_UUID(GGS_RIOT_UUID_LED_CHAR, &ggs_uuid_riot_base),
        .perm     = (BLE_ATT_READ | BLE_ATT_WRITE),
        .desc_cnt = 1,
        .desc     = _led_desc,
    },
#endif
#ifdef LED7_ON
    {
        .cb       = _led_cb,
        .arg      = (void *)7,
        .type     = GORM_UUID(GGS_RIOT_UUID_LED_CHAR, &ggs_uuid_riot_base),
        .perm     = (BLE_ATT_READ | BLE_ATT_WRITE),
        .desc_cnt = 1,
        .desc     = _led_desc,
    },
#endif
};

gorm_gatt_service_t ggs_riot_led_service = {
    .uuid  = GORM_UUID(GGS_RIOT_UUID_LED_SERVICE, &ggs_uuid_riot_base),
    .char_cnt = NUMOF_LED,
    .chars = _led_chars,
};
