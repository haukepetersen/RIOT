
#include <string.h>

#include "assert.h"
#include "xtimer.h"
#include "onewire.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

#define T_RESET_HOLD        (480)
#define T_RESET_SAMPLE      (70)

#define T_RW_PULSE          (3)
#define T_W_HOLD            (57)
#define T_W_RECOVER         (10)
#define T_R_SAMPLE          (7)
#define T_R_RECOVER         (55)


static int hex2int(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    }
    else if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    }
    else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    }
    else {
        return -1;
    }
}

static char int2hex(int num)
{
    if (num >= 0 && num <= 9) {
        return (char)(num + '0');
    }
    else if (num >= 10 && num <= 15) {
        return (char)(num + 'a' - 10);
    }
    else {
        return '?';
    }
}

static inline int read_bit(onewire_t *owi)
{
    int in;

    gpio_clear(owi->pin);
    xtimer_usleep(T_RW_PULSE);
    gpio_init(owi->pin, owi->imode);
    xtimer_usleep(T_R_SAMPLE);
    in = gpio_read(owi->pin);
    xtimer_usleep(T_R_RECOVER);
    gpio_init(owi->pin, owi->omode);
    gpio_set(owi->pin);

    return in;
}

static inline void write_bit(onewire_t *owi, int out)
{
    gpio_clear(owi->pin);
    if (out) {
        /* shift out a `1` */
        xtimer_usleep(T_RW_PULSE);
        gpio_set(owi->pin);
        xtimer_usleep(T_W_HOLD);
    }
    else {
        /* shift out a `0` */
        xtimer_usleep(T_RW_PULSE + T_W_HOLD);
        gpio_set(owi->pin);
    }
    xtimer_usleep(T_W_RECOVER);
}

void onewire_setup(onewire_t *owi, const onewire_params_t *params)
{
    owi->pin = params->pin;
    owi->omode = params->omode;
    owi->imode = params->imode;
}

int onewire_init(onewire_t *owi)
{
    if (gpio_init(owi->pin, owi->omode) != 0) {
        return ONEWIRE_ERR_PINCFG;
    }
    gpio_set(owi->pin);
    return ONEWIRE_OK;
}

int onewire_reset(onewire_t *owi, onewire_rom_t *rom)
{
    int res = ONEWIRE_OK;

    gpio_clear(owi->pin);
    xtimer_usleep(T_RESET_HOLD);
    gpio_set(owi->pin);

    xtimer_usleep(T_RESET_SAMPLE);
    gpio_init(owi->pin, owi->imode);
    if (gpio_read(owi->pin)) {
        res = ONEWIRE_NODEV;
    }

    xtimer_usleep(T_RESET_HOLD - T_RESET_SAMPLE);
    gpio_init(owi->pin, owi->omode);
    gpio_set(owi->pin);

    if ((res == ONEWIRE_OK) && (om != NULL)) {
        onewire_write_byte(owi, ONEWIRE_ROM_MATCH);
        onewire_write(owi, om->u8, sizeof(onewire_rom_t));
    }

    return res;
}

int onewire_search_first(onewire_t *owi, onewire_rom_t *rom)
{
    if (onewire_reset(owi) == ONEWIRE_NODEV) {
        return ONEWIRE_NODEV;
    }

    /* read ROM command */
    memset(rom, 0, sizeof(onewire_romt_t));
    onewire_write_byte(owi, ONEWIRE_ROM_READ);
    onewire_read(owi, rom->u8, sizeof(onewire_rom_t));

    return ONEWIRE_OK;
}

int onewire_search_next(onewire_t *owi, onewire_rom_t *rom)
{
    int newdev = 0;
    onewire_rom_t newrom;

    memset(&newrom, 0, sizeof(newrom));

    /* send reset condition on the bus */
    if (onewire_reset(owi) != ONEWIRE_OK) {
        return ONEWIRE_NODEV;
    }

    /* issue the search ROM command */
    onewire_write_byte(owi, ONEWIRE_ROM_SEARCH);

    for (int b = 0; b < 8; b++) {
        for (int i = 0; i < 8; i++) {
            int bit1 = (read_bit(owi)) ? 1 : 0;
            int bit2 = (read_bit(owi)) ? 1 : 0;

            if (bit1 == bit2) {
                if ((!newdev) && (rom->u8[b] & (1 << i))) {
                    bit1 = 1;
                }
                else if (!newdev) {
                    newdev = 1;
                }
            }

            newrom.u8[b] |= (bit1 << i);
            write_bit(owi, bit1);
        }
    }

    if (memcmp(&newrom, rom, sizeof(newrom)) == 0) {
        return ONEWIRE_NODEV;
    }

    memcpy(rom, &newrom, sizeof(newrom));
    return ONEWIRE_OK;
}

void onewire_read(onewire_t *owi, uint8_t *data, size_t len)
{
    for (unsigned pos = 0; pos < (unsigned)len; pos ++) {
        uint8_t tmp = 0;

        /* read the next byte from the bus */
        for (int i = 0; i < 8; i++) {
            tmp >>= 1;
            if (read_bit(owi)) {
                tmp |= 0x80;
            }
        }

        /* remember result */
        data[pos] = tmp;
    }
}

void onewire_write(onewire_t *owi, const uint8_t *data, size_t len)
{
    for (unsigned pos = 0; pos < (unsigned)len; pos ++) {

        for (int i = 0; i < 8; i++) {
            write_bit(owi, (data[pos] & (1 << i)));
        }
    }
}

// from https://lentz.com.au/blog/calculating-crc-with-a-tiny-32-entry-lookup-table
static const uint8_t _crc8_lut[] = {
    0x00, 0x5E, 0xBC, 0xE2, 0x61, 0x3F, 0xDD, 0x83,
    0xC2, 0x9C, 0x7E, 0x20, 0xA3, 0xFD, 0x1F, 0x41,
    0x00, 0x9D, 0x23, 0xBE, 0x46, 0xDB, 0x65, 0xF8,
    0x8C, 0x11, 0xAF, 0x32, 0xCA, 0x57, 0xE9, 0x74
};

// from https://lentz.com.au/blog/calculating-crc-with-a-tiny-32-entry-lookup-table
uint8_t onewire_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0;

    for (unsigned i = 0; i < len; i++) {
        crc ^= data[i];
        crc = (_crc8_lut + (crc & 0x0f)) ^
              (_crc8_lut + 16 + ((crc >> 4) & 0x0f));
    }
    return crc;
}

int onewire_rom_from_str(onewire_rom_t *rom, const char *str)
{
    if (strlen(str) != 16) {
        return ONEWIRE_ERR_ROMSTR;
    }

    memset(rom->u8, 0, sizeof(onewire_rom_t));
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 2; j++) {
            int digit = hex2int(str[(i * 2) + j]);
            if (digit < 0) {
                return ONEWIRE_ERR_ROMSTR;
            }
            rom->u8[i] |= (digit << ((1 - j) * 4));
        }
    }

    return ONEWIRE_OK;
}

void onewire_rom_to_str(onewire_rom_t *rom, char *str)
{
    assert(rom && str);

    for (int b = 0; b < 7; b++) {
        *(str++) = int2hex(rom->u8[b] & 0x0f);
        *(str++) = int2hex(rom->u8[b] >> 4);
    }
    *str = '\0';
}
