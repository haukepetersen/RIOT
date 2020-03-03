
#include <stdio.h>

#include "fmt.h"
#include "assert.h"
#include "onewire.h"

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

// static char int2hex(int num)
// {
//     if (num >= 0 && num <= 9) {
//         return (char)(num + '0');
//     }
//     else if (num >= 10 && num <= 15) {
//         return (char)(num + 'a' - 10);
//     }
//     else {
//         return '?';
//     }
// }



int onewire_rom_from_str(onewire_rom_t *rom, const char *str)
{
    assert(rom);
    assert(str);

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

void onewire_rom_sprint(char *str, const onewire_rom_t *rom)
{
    assert(rom);
    assert(str);

    fmt_bytes_hex(str, rom->u8, sizeof(onewire_rom_t));
    str[sizeof(onewire_rom_t)] = '\0';
}

void onewire_rom_print(const onewire_rom_t *rom)
{
    assert(rom);

    for (unsigned i = 0; i < sizeof(onewire_rom_t); i++) {
        printf("%02x", (int)rom->u8[i]);
    }
}
