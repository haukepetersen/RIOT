
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



static inline int read_bit(const onewire_t *owi)
{
    gpio_clear(owi->pin);
    xtimer_usleep(T_RW_PULSE);
    gpio_init(owi->pin, owi->imode);
    xtimer_usleep(T_R_SAMPLE);
    int in = (gpio_read(owi->pin)) ? 1 : 0;
    xtimer_usleep(T_R_RECOVER);
    gpio_init(owi->pin, owi->omode);
    gpio_set(owi->pin);

    return in;
}

static inline void write_bit(const onewire_t *owi, int out)
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

int onewire_init(onewire_t *owi, const onewire_params_t *params)
{
    owi->pin = params->pin;

    owi->omode = params->pin_mode;
    owi->imode = (owi->omode == GPIO_OD_PU) ? GPIO_IN_PU : GPIO_IN;

    if (gpio_init(owi->pin, owi->omode) != 0) {
        return ONEWIRE_ERR_PINCFG;
    }
    gpio_set(owi->pin);
    return ONEWIRE_OK;
}

int onewire_reset(const onewire_t *owi, const onewire_rom_t *rom)
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

    if ((res == ONEWIRE_OK) && (rom != NULL)) {
        onewire_write_byte(owi, ONEWIRE_ROM_MATCH);
        onewire_write(owi, rom->u8, sizeof(onewire_rom_t));
    }

    return res;
}

void onewire_read(const onewire_t *owi, uint8_t *data, size_t len)
{
    for (unsigned pos = 0; pos < (unsigned)len; pos++) {
        /* read the next byte from the bus */
        uint8_t tmp = 0;
        for (int i = 0; i < 8; i++) {
            tmp |= (read_bit(owi) << i);
        }
        data[pos] = tmp;
    }
}

void onewire_write(const onewire_t *owi, const uint8_t *data, size_t len)
{
    for (unsigned pos = 0; pos < (unsigned)len; pos ++) {

        for (int i = 0; i < 8; i++) {
            write_bit(owi, (data[pos] & (1 << i)));
        }
    }
}

// int onewire_search_first(const onewire_t *owi, onewire_rom_t *rom)
// {
//     memset(rom, 0, sizeof(onewire_rom_t));

//     if (onewire_reset(owi, NULL) == ONEWIRE_NODEV) {
//         return ONEWIRE_NODEV;
//     }

//     /* read ROM command */
//     onewire_write_byte(owi, ONEWIRE_ROM_READ);
//     onewire_read(owi, rom->u8, sizeof(onewire_rom_t));

//     return ONEWIRE_OK;
// }

static uint8_t b1[64];
static uint8_t b2[64];
static uint8_t w0[64];

int onewire_search(const onewire_t *owi, onewire_rom_t *rom, int *state)
{
    onewire_rom_t newrom;

    memset(&newrom, 0, sizeof(newrom));

    /* send reset condition on the bus */
    if (onewire_reset(owi, NULL) != ONEWIRE_OK) {
        return ONEWIRE_NODEV;
    }

    /* issue the search ROM command */

    if (*state & 0x8000) {
        return ONEWIRE_NODEV;
    }

    int pos = 0;
    int marker = 0;
    onewire_write_byte(owi, ONEWIRE_ROM_SEARCH);

    for (unsigned b = 0; b < sizeof(onewire_rom_t); b++) {
        for (int i = 0; i < 8; i++) {
            int bit1 = (read_bit(owi)) ? 1 : 0;
            int bit2 = (read_bit(owi)) ? 1 : 0;


            b1[pos] = bit1;
            b2[pos] = bit2;

            if (bit1 == bit2) {
                if (pos == *state) {
                    bit1 = 1;
                }
                else if (pos > *state) {
                    bit1 = 0;
                    marker = pos;
                }
                else {
                    if (!(rom->u8[b] & (1 << i))) {
                        marker = pos;
                    }
                }
            }

            newrom.u8[b] |= (bit1 << i);
            write_bit(owi, bit1);
            pos++;
        }
    }

    *state = marker;
    if (*state == 0) {
        *state &= 0x8000;
    }

    return ONEWIRE_OK;



    // int last_conflict = -1;




    // for (unsigned b = 0; b < sizeof(onewire_rom_t); b++) {
    //     for (int i = 0; i < 8; i++) {
    //         int bit1 = (read_bit(owi)) ? 1 : 0;
    //         int bit2 = (read_bit(owi)) ? 1 : 0;

    //         b1[pos] = bit1;
    //         b2[pos] = bit2;

    //         int known = (rom->u8[b] & (1 << i)) ? 1 : 0;
    //         oldrom[pos] = (uint8_t)known;

    //         if (bit1 == bit2) {     /* collision at bit position */
    //             if (pos > *state) {
    //                 bit1 = 1;
    //                 *state = pos;
    //             }
    //             else if (pos == *state) {
    //                 if (last_conflict == -1) {
    //                     return ONEWIRE_NODEV;
    //                 }
    //                 bit1 = 0;
    //                 *state = last_conflict;
    //             }
    //             else {
    //                 last_conflict = pos;
    //             }
    //         }

    //         w0[pos++] = bit1;

    //         newrom.u8[b] |= (bit1 << i);
    //         write_bit(owi, bit1);
    //     }
    // }

    printf("\nbit 1 ");
    for (int i = 0; i < 64; i++) {
        printf("%i", b1[i]);
    }
    printf("\nbit 2 ");
    for (int i = 0; i < 64; i++) {
        printf("%i", b2[i]);
    }
    printf("\ndiff  ");
    for (int i = 0; i < 64; i++) {
        printf("%i", b1[i] ^ b2[i]);
    }
    printf("\nwrite ");
    for (int i = 0; i < 64; i++) {
        printf("%i", w0[i]);
    }
    puts("");

    if (memcmp(&newrom, rom, sizeof(newrom)) == 0) {
        return ONEWIRE_NODEV;
    }

    memcpy(rom, &newrom, sizeof(newrom));
    return ONEWIRE_OK;
}
