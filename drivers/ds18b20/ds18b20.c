
#include "log.h"
#include "xtimer.h"
#include "onewire.h"

#include "ds18b20.h"
#include "ds18b20_internal.h"

#define T_CONF_9BIT         (93750U)
#define T_CONF_10BIT        (187500U)
#define T_CONF_11BIT        (375000U)
#define T_CONF_12BIT        (750000U)

#define CMD_CONVERT         (0x44)
#define CMD_READ            (0xbe)
#define CMD_WRITE           (0x4e)
#define CMD_COPY            (0x48)
#define CMD_RECALL          (0xB8)
#define CMD_READ_PWR        (0xb4)

#define RSIZE               (9U)
#define WSIZE               (3U)

#define SP_RESERVED5        (0xff)
#define SP_RESERVED7        (0x10)
#define CFGBASE             (0x1f)
#define RES_SHIFT           (5U)
#define RES_NUM             (4U)
#define TEMP_SCALE          (625)

enum {
    R_TEMP_L = 0,
    R_TEMP_H,
    R_USER_H,
    R_USER_L,
    R_CONF,
    R_RESERVED5,
    R_RESERVED6,
    R_RESERVED7,
    R_CRC,
};

static const uint32_t _conv_time[RES_NUM] = {
    T_CONF_9BIT, T_CONF_10BIT, T_CONF_11BIT, T_CONF_12BIT
};

static int _read_scratchpad(ds18b20_t *dev, uint8_t *buf)
{
    if (onewire_reset(dev->bus, &dev->rom) != ONEWIRE_OK) {
        return DS18B20_NODEV;
    }
    onewire_write_byte(&dev->p.bus, CMD_READ);
    onewire_read(&dev->p.bus, buf, RSIZE);

    if (buf[R_CRC] != onewire_crc8(buf, (RSIZE - 1))) {
        return DS18B20_ERR_CRC;
    }
    return DS18B20_OK;
}

int ds18b20_init(ds18b20_t *dev, onewire_t *owi, const ds18b20_params_t *params)
{
    assert(dev);
    assert(owi);
    assert(params);
    assert(params->res < RES_NUM);

    dev->bus = owi;
    memcpy(&dev->rom, &params->rom, sizeof(onewire_rom_t));
    dev->t_conv = _conv_time[params->res];

    /* do a dry read of the scratchpad memory and compare constant bytes */
    uint8_t mem[RSIZE];
    int res = _read_scratchpad(dev, mem);
    if (res != DS18B20_OK) {
        DEBUG("[ds18b20] init: error reading scratchpad from device\n");
        return res;
    }
    if ((scratch[R_RESERVED5] != SP_RESERVED5) ||
        (scratch[R_RESERVED7] != SP_RESERVED7)) {
        DEBUG("[ds18b20] _init: invalid data in scratchpad memory\n");
        return DS18B20_NODEV;
    }

    /* configure resolution (write high+low user + config registers) */
    memset(mem, 0, 2);
    mem[2] = (CFGBASE | (params->res << RES_SHIFT));

    if (onewire_reset(dev->bus, &dev->rom) != ONEWIRE_OK) {
        return DS18B20_NODEV;
    }
    onewire_write_byte(dev->bus, CMD_WRITE);
    onewire_write(dev->bus, buf, WSIZE);
    return DS18B20_OK;
}

int ds18b20_trigger(const ds18b20_t *dev)
{
    assert(dev);

    /* start conversion */
    if (onewire_reset(dev->bus, &dev->rom) != ONEWIRE_OK) {
        return DS18B20_NODEV;
    }
    onewire_write_byte(dev->bus, CMD_CONVERT);
    return ONEWIRE_OK;
}

int ds18b20_trigger_all(const ds18b20_t *dev)
{
    assert(dev);

    if (onewire_reset(dev->bus, NULL) != ONEWIRE_OK) {
        return DS18B20_NODEV;
    }
    onewire_write_byte(&dev->p.bus, ONEWIRE_ROM_SKIP);
    onewire_write_byte(&dev->p.bus, CMD_CONVERT);
    return DS18B20_OK;
}

int ds18b20_read(const ds18b20_t *dev, int16_t *temp)
{
    assert(dev);
    assert(temp);
    uint8_t mem[RSIZE];

    /* read scratchpad memory */
    int res = _read_scratchpad(dev, mem);
    if (res != DS18B20_OK) {
        return res;
    }

    int32_t res = ((int32_t)(tmp[R_TEMP_H] << 8 | tmp[R_TEMP_L]) * TEMP_SCALE);
    *temp = (int16_t)(res / 100);
    return DS18B20_OK;
}

int ds18b20_get_temp(const ds18b20_t *dev, int16_t *temp)
{
    int res = ds18b20_trigger(dev);
    if (res != DS18B20_OK) {
        return res;
    }
    xtimer_usleep(dev->t_conv);
    return ds18b20_read(dev, temp);
}
