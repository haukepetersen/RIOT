
#include <stdint.h>

#include "cpu.h"
#include "thread.h"
#include "sched.h"
#include "periph/spi.h"
#include "periph_conf.h"

/* compile file only if a SPI device is configured */
#if SPI_0_EN

int spi_init_master(spi_t dev, spi_conf_t conf, spi_speed_t speed)
{
    if (dev != SPI_0) {
        return -1;
    }

    /* power on SPI device */
    SPI_0_CLKEN();
    /* set peripheral clock to CCLK / 4 (24MHz) */
    LPC_SC->PCLKSEL0 &= ~(0x03 << 16);
    /* configure pins */
    spi_conf_pins(dev);
    /* configure polarity and phase and enable master mode */
    SPI_0_DEV->SPCR = ((conf & 0x3) << 3) | (1 << 5);
    /* set SPI clock speed */
    switch (speed) {
        case SPI_SPEED_100KHZ:
            SPI_0_DEV->SPCCR = 240;         /* actual bus speed: 100KHz */
            break;
        case SPI_SPEED_400KHZ:
            SPI_0_DEV->SPCCR = 60;          /* actual bus speed: 400KHz */
            break;
        case SPI_SPEED_1MHZ:
            SPI_0_DEV->SPCCR = 24;          /* actual bus speed: 1MHz */
            break;
        case SPI_SPEED_5MHZ:
            SPI_0_DEV->SPCCR = 5;          /* actual bus speed: 4.8MHz */
            break;
        case SPI_SPEED_10MHZ:
            SPI_0_DEV->SPCCR = 2;          /* actual bus speed: 12MHz */
            break;
        default:
            return -2;
    }
    /* enable SPI device */
    SPI_0_DEV->SPCR |= (1 << 7);
}

int spi_init_slave(spi_t dev, spi_conf_t conf, char (*cb)(char data))
{
    /* TODO not implemented, yet */
}

int spi_conf_pins(spi_t dev)
{
    if (dev != SPI_0) {
        return -1;
    }

    /* configure MISO pin -> no pull resistors */
    SPI_0_MISO_PINSEL &= ~(0x3 << (SPI_0_MISO_PIN * 2));
    SPI_0_MISO_PINSEL |= (SPI_0_MISO_AF << (SPI_0_MISO_PIN * 2));
    SPI_0_MISO_PINMODE &= ~(0x3 << (SPI_0_MISO_PIN * 2));
    SPI_0_MISO_PINMODE |= (0x2 << (SPI_0_MISO_PIN * 2));
    /* configure MISO pin -> no pull resistors */
    SPI_0_MOSI_PINSEL &= ~(0x3 << (SPI_0_MOSI_PIN * 2));
    SPI_0_MOSI_PINSEL |= (SPI_0_MOSI_AF << (SPI_0_MOSI_PIN * 2));
    SPI_0_MOSI_PINMODE &= ~(0x3 << (SPI_0_MOSI_PIN * 2));
    SPI_0_MOSI_PINMODE |= (0x2 << (SPI_0_MOSI_PIN * 2));
    /* configure MISO pin -> no pull resistors */
    SPI_0_SCK_PINSEL &= ~(0x3 << (SPI_0_SCK_PIN * 2));
    SPI_0_SCK_PINSEL |= (SPI_0_SCK_AF << (SPI_0_SCK_PIN * 2));
    SPI_0_SCK_PINMODE &= ~(0x3 << (SPI_0_SCK_PIN * 2));
    SPI_0_SCK_PINMODE |= (0x2 << (SPI_0_SCK_PIN * 2));

    return 0;
}

int spi_transfer_byte(spi_t dev, char out, char *in)
{
    return spi_transfer_bytes(dev, &out, in, 1);
}

int spi_transfer_bytes(spi_t dev, char *out, char *in, unsigned int length)
{
    if (dev != SPI_0) {
        return -1;
    }

    for (unsigned int i = 0; i < length; i++) {
        char data;
        /* figure out what to send out */
        if (out) {
            data = out[i];
        }
        else {
            data = 0;
        }
        /* put data into shift register */
        SPI_0_DEV->SPDR = (uint8_t)data;
        /* wait for transfer to be completed */
        while(!(SPI_0_DEV->SPSR & (1 << 7)));       /* wait for SPIF bit to be set */
        /* get result if wanted */
        if (in) {
            in[i] = (char)SPI_0_DEV->SPDR;
        }
    }

    return length;
}

int spi_transfer_reg(spi_t dev, uint8_t reg, char out, char *in)
{
    return spi_transfer_regs(dev, reg, &out, in, 1);
}

int spi_transfer_regs(spi_t dev, uint8_t reg, char *out, char *in, unsigned int length)
{
    if (dev != SPI_0) {
        return -1;
    }

    spi_transfer_bytes(dev, &reg, NULL, 1);
    return spi_transfer_bytes(dev, out, in, length);
}

void spi_transmission_begin(spi_t dev, char reset_val)
{
    if (dev == SPI_0) {
        SPI_0_DEV->SPDR = (uint8_t)reset_val;
    }
}

void spi_poweron(spi_t dev)
{
    if (dev == SPI_0) {
        SPI_0_CLKEN();
    }
}

void spi_poweroff(spi_t dev)
{
    if (dev == SPI_0) {
        SPI_0_CLKDIS();
    }
}

#endif /* SPI_0_EN */
