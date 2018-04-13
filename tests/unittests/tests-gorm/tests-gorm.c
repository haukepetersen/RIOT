/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup  unittests
 * @{
 *
 * @file
 * @brief       Implementations of unit tests for the SAUL registry
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <string.h>

#include "embUnit/embUnit.h"
#include "tests-gorm.h"

#include "net/gorm/gatt.h"
#include "net/gorm/gatt/tab.h"
#include "net/gorm/uuid.h"
#include "net/gorm/buf.h"

/* define UUIDs used by this test */
#define UUID_S1             (0xaffe)
#define UUID_S1_C1          (0x1234)
#define UUID_S1_C2          (0x1235)
#define UUID_S1_C3          (0x1236)
#define UUID_S2             (0xcafe)
#define UUID_S2_C1          (0x5588)
#define UUID_S3             (0xfee1)
#define UUID_S3_C1          (0x8989)

#define UUID_FAIL1          (0xa3d2)
#define UUID_FAIL2          (0xaff3)
#define UUID_FAIL3          (0x0100)

/* these are the handles that should be assigned by the GATT table... */
#define HANDLE_FIRST_16     (0x0000)
#define HANDLE_FIRST_128    (0x8000)

#define HANDLE_S_GAP        (0x0100)
#define HANDLE_S_ATT        (0x0200)
#define HANDLE_S1           (0x8100)
#define HANDLE_S2           (0x8200)
#define HANDLE_S3           (0x0300)

#define HANDLE_GAP_C3       (0x0130)
#define HANDLE_GAP_C3_VAL   (0x0138)
#define HANDLE_S1_C1        (0x8110)
#define HANDLE_S1_C1_VAL    (0x8118)
#define HANDLE_S1_C2        (0x8120)
#define HANDLE_S1_C2_VAL    (0x8128)
#define HANDLE_S1_C3        (0x8130)
#define HANDLE_S1_C3_VAL    (0x8138)
#define HANDLE_S2_C1        (0x8210)
#define HANDLE_S2_C1_VAL    (0x8218)
#define HANDLE_S3_C1        (0x0310)
#define HANDLE_S3_C1_VAL    (0x0318)

#define HANDLE_FAIL1        (0x0400)
#define HANDLE_FAIL2        (0x0101)
#define HANDLE_FAIL3        (0x2800)


// size_t _dummy_cb(const gorm_gatt_char_t *characteristic,
//                 uint8_t method, uint8_t *buf, size_t buf_len);

// static const gorm_uuid_base_t test_uuid = {{
//     0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//     0x0a, 0x0f, 0x0f, 0x0e,
//     0x00, 0x00, 0x52, 0x49, 0x4f, 0x54
// }};

// static gorm_gatt_entry_t e1;
// static gorm_gatt_entry_t e2;
// static gorm_gatt_entry_t e3;

// static const gorm_gatt_service_t s1 = {
//     .uuid = GORM_UUID(UUID_S1, &test_uuid),
//     .char_cnt = 3,
//     .chars = (gorm_gatt_char_t[]){
//         {
//             .type = GORM_UUID(UUID_S1_C1, &test_uuid),
//             .perm = GORM_ATT_READABLE,
//             .cb   = _dummy_cb,
//         },
//         {
//             .type = GORM_UUID(UUID_S1_C2, NULL),
//             .perm = GORM_ATT_READABLE,
//             .cb   = _dummy_cb,
//         },
//         {
//             .type = GORM_UUID(UUID_S1_C3, &test_uuid),
//             .perm = GORM_ATT_READABLE,
//             .cb   = _dummy_cb,
//         },
//     },
// };

// static const gorm_gatt_service_t s2 = {
//     .uuid = GORM_UUID(UUID_S2, &test_uuid),
//     .char_cnt = 1,
//     .chars = (gorm_gatt_char_t[]){
//         {
//             .type = GORM_UUID(UUID_S2_C1, NULL),
//             .perm = GORM_ATT_READABLE,
//             .cb   = _dummy_cb,
//         },
//     },
// };

// static const gorm_gatt_service_t s3 = {
//     .uuid = GORM_UUID(UUID_S3, NULL),
//     .char_cnt = 1,
//     .chars = (gorm_gatt_char_t[]){
//         {
//             .type = GORM_UUID(UUID_S3_C1, &test_uuid),
//             .perm = GORM_ATT_READABLE,
//             .cb   = _dummy_cb,
//         },
//     },
// };

// static char writebuf[10];

// size_t _dummy_cb(const gorm_gatt_char_t *characteristic,
//                  uint8_t op, uint8_t *buf, size_t buf_len)
// {
//     const char *text = "RIOT";

//     if (!characteristic || !buf || (buf_len < 5) || (buf_len > 10)) {
//         return 0;
//     }

//     switch (op) {
//         case GORM_GATT_READ:
//             memcpy(buf, text, 5);
//             return 5;
//         case GORM_GATT_WRITE:
//             memcpy(writebuf, buf, buf_len);
//             return buf_len;
//         default:
//             return 0;
//     }
// }

// static void test_service_reg(void)
// {
//     gorm_gatt_tab_init();
//     gorm_gatt_entry_t *s;

//     /* test initial state */
//     s = gorm_gatt_tab_find_service(HANDLE_FIRST_16);
//     TEST_ASSERT_NOT_NULL(s);
//     TEST_ASSERT_NULL(s->service->uuid.base);
//     TEST_ASSERT_EQUAL_INT((int)GORM_UUID_GAP, (int)s->service->uuid.uuid16);

//     s = gorm_gatt_tab_get_service(HANDLE_S_GAP);
//     TEST_ASSERT_NOT_NULL(s);
//     TEST_ASSERT_NULL(s->service->uuid.base);
//     TEST_ASSERT_EQUAL_INT((int)GORM_UUID_GAP, (int)s->service->uuid.uuid16);

//     s = gorm_gatt_tab_get_service(HANDLE_S_ATT);
//     TEST_ASSERT_NOT_NULL(s);
//     TEST_ASSERT_NULL(s->service->uuid.base);
//     TEST_ASSERT_EQUAL_INT((int)GORM_UUID_ATT, (int)s->service->uuid.uuid16);

//     s = gorm_gatt_tab_find_service(HANDLE_FIRST_128);
//     TEST_ASSERT_NULL(s);

//     /* insert new services */
//     gorm_gatt_tab_reg_service(&e1, &s1);
//     gorm_gatt_tab_reg_service(&e2, &s2);
//     gorm_gatt_tab_reg_service(&e3, &s3);

//     /* TODO: REMOVE */
//     gorm_gatt_tab_print();

//     /* make sure initial state is still valid */
//     s = gorm_gatt_tab_find_service(HANDLE_FIRST_16);
//     TEST_ASSERT_NOT_NULL(s);
//     TEST_ASSERT_NULL(s->service->uuid.base);
//     TEST_ASSERT_EQUAL_INT((int)GORM_UUID_GAP, (int)s->service->uuid.uuid16);

//     s = gorm_gatt_tab_get_service(HANDLE_S_GAP);
//     TEST_ASSERT_NOT_NULL(s);
//     TEST_ASSERT_NULL(s->service->uuid.base);
//     TEST_ASSERT_EQUAL_INT((int)GORM_UUID_GAP, (int)s->service->uuid.uuid16);

//     s = gorm_gatt_tab_get_service(HANDLE_S_ATT);
//     TEST_ASSERT_NOT_NULL(s);
//     TEST_ASSERT_NULL(s->service->uuid.base);
//     TEST_ASSERT_EQUAL_INT((int)GORM_UUID_ATT, (int)s->service->uuid.uuid16);

//     /* and test the rest of the entries */
//     s = gorm_gatt_tab_find_service(HANDLE_FIRST_128);
//     TEST_ASSERT_NOT_NULL(s);
//     TEST_ASSERT_NOT_NULL(s->service->uuid.base);
//     TEST_ASSERT_EQUAL_INT((int)UUID_S1, (int)s->service->uuid.uuid16);

//     s = gorm_gatt_tab_get_service(HANDLE_S1);
//     TEST_ASSERT_NOT_NULL(s);
//     TEST_ASSERT_NOT_NULL(s->service->uuid.base);
//     TEST_ASSERT_EQUAL_INT((int)UUID_S1, (int)s->service->uuid.uuid16);

//     s = gorm_gatt_tab_get_service(HANDLE_S2);
//     TEST_ASSERT_NOT_NULL(s);
//     TEST_ASSERT_NOT_NULL(s->service->uuid.base);
//     TEST_ASSERT_EQUAL_INT((int)UUID_S2, (int)s->service->uuid.uuid16);

//     s = gorm_gatt_tab_get_service(HANDLE_S3);
//     TEST_ASSERT_NOT_NULL(s);
//     TEST_ASSERT_NULL(s->service->uuid.base);
//     TEST_ASSERT_EQUAL_INT((int)UUID_S3, (int)s->service->uuid.uuid16);

//     /* test to read out some characteristics */
//     size_t len = 10;
//     uint8_t buf[10];

//     TEST_ASSERT_EQUAL_INT(-1, (int)gorm_gatt_tab_read_val(HANDLE_FAIL1, buf, len));
//     TEST_ASSERT_EQUAL_INT(-1, (int)gorm_gatt_tab_read_val(HANDLE_FAIL2, buf, len));
//     TEST_ASSERT_EQUAL_INT(-1, (int)gorm_gatt_tab_read_val(HANDLE_FAIL3, buf, len));
//     TEST_ASSERT_EQUAL_INT(-1, (int)gorm_gatt_tab_read_val(HANDLE_S1, buf, len));
//     TEST_ASSERT_EQUAL_INT(-1, (int)gorm_gatt_tab_read_val(HANDLE_S2, buf, len));
//     TEST_ASSERT_EQUAL_INT(-1, (int)gorm_gatt_tab_read_val(HANDLE_S3, buf, len));

//     TEST_ASSERT_EQUAL_INT(-1, (int)gorm_gatt_tab_read_val(HANDLE_S1_C1, buf, len));
//     TEST_ASSERT_EQUAL_INT(-1, (int)gorm_gatt_tab_read_val(HANDLE_S1_C2, buf, len));
//     TEST_ASSERT_EQUAL_INT(-1, (int)gorm_gatt_tab_read_val(HANDLE_S2_C1, buf, len));
//     TEST_ASSERT_EQUAL_INT(-1, (int)gorm_gatt_tab_read_val(HANDLE_S3_C1, buf, len));

//     TEST_ASSERT_EQUAL_INT(5, (int)gorm_gatt_tab_read_val(HANDLE_S1_C1_VAL, buf, len));
//     TEST_ASSERT_EQUAL_STRING("RIOT", (char *)buf);
//     TEST_ASSERT_EQUAL_INT(5, (int)gorm_gatt_tab_read_val(HANDLE_S1_C2_VAL, buf, len));
//     TEST_ASSERT_EQUAL_STRING("RIOT", (char *)buf);
//     TEST_ASSERT_EQUAL_INT(5, (int)gorm_gatt_tab_read_val(HANDLE_S1_C3_VAL, buf, len));
//     TEST_ASSERT_EQUAL_STRING("RIOT", (char *)buf);
//     TEST_ASSERT_EQUAL_INT(5, (int)gorm_gatt_tab_read_val(HANDLE_S2_C1_VAL, buf, len));
//     TEST_ASSERT_EQUAL_STRING("RIOT", (char *)buf);
//     TEST_ASSERT_EQUAL_INT(5, (int)gorm_gatt_tab_read_val(HANDLE_S3_C1_VAL, buf, len));
//     TEST_ASSERT_EQUAL_STRING("RIOT", (char *)buf);

//     memcpy(buf, "bier", 5);
//     TEST_ASSERT_EQUAL_INT(5, (int)gorm_gatt_tab_write_val(HANDLE_S1_C1_VAL, buf, 5));
//     TEST_ASSERT_EQUAL_STRING("bier", (char *)writebuf);
//     memcpy(buf, "pharisaer", 10);
//     TEST_ASSERT_EQUAL_INT(10, (int)gorm_gatt_tab_write_val(HANDLE_S1_C1_VAL, buf, 10));
//     TEST_ASSERT_EQUAL_STRING("pharisaer", (char *)writebuf);
//     memcpy(buf, "futjes", 7);
//     TEST_ASSERT_EQUAL_INT(7, (int)gorm_gatt_tab_write_val(HANDLE_S1_C1_VAL, buf, 7));
//     TEST_ASSERT_EQUAL_STRING("futjes", (char *)writebuf);

// }

#define POOL_MEM1_LEN       (5U * sizeof(gorm_buf_t))
#define POOL_MEM2_LEN       (1U)
#define POOL_MEM3_LEN       (sizeof(gorm_buf_t))
#define POOL_MEM4_LEN       (sizeof(gorm_buf_t) - 1)
#define POOL_MEM5_LEN       ((3U * sizeof(gorm_buf_t)) + 2)
#define POOL_MEM6_LEN       (2U * sizeof(gorm_buf_t))

static uint8_t mem1[POOL_MEM1_LEN];
static uint8_t mem2[POOL_MEM1_LEN];
static uint8_t mem3[POOL_MEM1_LEN];
static uint8_t mem4[POOL_MEM1_LEN];
static uint8_t mem5[POOL_MEM1_LEN];
static uint8_t mem6[POOL_MEM1_LEN];

static void test_buf(void)
{
    gorm_buf_addmem(mem1, POOL_MEM1_LEN);
    TEST_ASSERT_EQUAL_INT(5, (int)gorm_buf_count(&gorm_buf_pool));
    gorm_buf_addmem(mem2, POOL_MEM2_LEN);
    TEST_ASSERT_EQUAL_INT(5, (int)gorm_buf_count(&gorm_buf_pool));
    gorm_buf_addmem(mem3, POOL_MEM3_LEN);
    TEST_ASSERT_EQUAL_INT(6, (int)gorm_buf_count(&gorm_buf_pool));
    gorm_buf_addmem(mem4, POOL_MEM4_LEN);
    TEST_ASSERT_EQUAL_INT(6, (int)gorm_buf_count(&gorm_buf_pool));
    gorm_buf_addmem(mem5, POOL_MEM5_LEN);
    TEST_ASSERT_EQUAL_INT(9, (int)gorm_buf_count(&gorm_buf_pool));
    gorm_buf_addmem(mem6, POOL_MEM6_LEN);
    TEST_ASSERT_EQUAL_INT(11, (int)gorm_buf_count(&gorm_buf_pool));

    gorm_buf_t *buf;
    gorm_bufq_t tmp = GORM_BUFQ_INIT;

    for (int i = 11; i > 0; i--) {
        buf = gorm_buf_get();
        TEST_ASSERT_NOT_NULL(buf);
        TEST_ASSERT_NULL(buf->next);
        TEST_ASSERT_EQUAL_INT((i - 1), (int)gorm_buf_count(&gorm_buf_pool));
        gorm_buf_enq(&tmp, buf);
        TEST_ASSERT_EQUAL_INT((12 - i), (int)gorm_buf_count(&tmp));
    }

    buf = gorm_buf_get();
    TEST_ASSERT_NULL(buf);
    TEST_ASSERT_EQUAL_INT(0, (int)gorm_buf_count(&gorm_buf_pool));
    TEST_ASSERT_EQUAL_INT(11, (int)gorm_buf_count(&tmp));

    for (int i = 0; i < 11; i++) {
        buf = gorm_buf_deq(&tmp);
        TEST_ASSERT_NOT_NULL(buf);
        TEST_ASSERT_EQUAL_INT((10 - i), (int)gorm_buf_count(&tmp));
        TEST_ASSERT_EQUAL_INT(i, (int)gorm_buf_count(&gorm_buf_pool));
        gorm_buf_return(buf);
        TEST_ASSERT_EQUAL_INT((i + 1), (int)gorm_buf_count(&gorm_buf_pool));
    }

    buf = gorm_buf_deq(&tmp);
    TEST_ASSERT_NULL(buf);
    TEST_ASSERT_EQUAL_INT(0, (int)gorm_buf_count(&tmp));
}

Test *tests_gorm_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_buf),
        // new_TestFixture(test_service_reg),
    };

    EMB_UNIT_TESTCALLER(gorm_tests, NULL, NULL, fixtures);

    return (Test *)&gorm_tests;
}

void tests_gorm(void)
{
    TESTS_RUN(tests_gorm_tests());
}
