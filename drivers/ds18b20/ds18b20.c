
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

#define SP_RESERVED5        (0xff)
#define SP_RESERVED7        (0x10)

static int reset(ds18b20_t *dev)
{
    if (onewire_reset(&dev->p.bus) != ONEWIRE_OK) {
        return -1;
    }
    onewire_write_byte(&dev->p.bus, ONEWIRE_ROM_MATCH);
    onewire_write(&dev->p.bus, dev->p.rom.u8, 8);
    return 0;
}

static int read(ds18b20_t *dev, uint8_t *buf, size_t len)
{
    if (reset(dev) < 0) {
        return -1;
    }
    onewire_write_byte(&dev->p.bus, CMD_READ);
    onewire_read(&dev->p.bus, buf, len);
    return (int)len;
}

static int write(ds18b20_t *dev, uint8_t *buf)
{
    if (reset(dev) < 0) {
        return -1;
    }

    return 1;
}

void ds18b20_setup(ds18b20_t *dev, const ds18b20_params_t *params)
{
    memcpy(&dev->p, params, sizeof(ds18b20_params_t));
    dev->t_conv = 0;
}

int ds18b20_init(ds18b20_t *dev)
{
    uint8_t scratch[9];

    /* set conversion time corresponding to configured resolution */
    switch (dev->p.res) {
        case DS18B20_RES_9BIT:  dev->t_conv = T_CONF_9BIT;  break;
        case DS18B20_RES_10BIT: dev->t_conv = T_CONF_10BIT; break;
        case DS18B20_RES_11BIT: dev->t_conv = T_CONF_11BIT; break;
        case DS18B20_RES_12BIT: dev->t_conv = T_CONF_12BIT; break;
        default:
            LOG_ERROR("[ds18b20] _init: invalid resolution value\n");
            return DS18B20_ERR_RES;
    }

    /* do a dry read of the scratchpad memory and compare constant bytes */
    if (read(dev, scratch, 9) < 0) {
        LOG_ERROR("[ds18b20] _init: no device responding on the bus\n");
    }
    if ((scratch[5] != SP_RESERVED5) || (scratch[7] != SP_RESERVED7)) {
        LOG_ERROR("[ds18b20] _init: no connection to given device\n");
        return DS18B20_NO_DEV;
    }

    /* configure resolution */
    uint8_t buf[] = { 0, 0, 0 };
    buf[2] = dev->p.res;
    write(dev, buf);

    return DS18B20_OK;
}

void ds18b20_trigger(ds18b20_t *dev)
{
    /* start conversion */
    reset(dev);
    onewire_write_byte(&dev->p.bus, CMD_CONVERT);
}

void ds18b20_trigger_all(ds18b20_t *dev)
{
    onewire_reset(&dev->p.bus);
    onewire_write_byte(&dev->p.bus, ONEWIRE_ROM_SKIP);
    onewire_write_byte(&dev->p.bus, CMD_CONVERT);
}

int16_t ds18b20_read(ds18b20_t *dev)
{
    uint8_t tmp[2];

    /* get results from last conversion */
    read(dev, tmp, 2);

    int32_t res = ((int32_t)(tmp[1] << 8 | tmp[0]) * 625);
    return (int16_t)(res / 10);
}

int16_t ds18b20_get_temp(ds18b20_t *dev)
{
    ds18b20_trigger(dev);
    xtimer_usleep(dev->t_conv);
    return ds18b20_read(dev);
}
