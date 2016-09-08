

#include "devabc.h"
#include "somenet.h"

DEV_DEF const somenet_params_t devs[] = {
    {
        .base.root  = &abcdev_root,
        DEV_NAME("ETH0")
        .spi        = SPI_DEV(0),
        .spi_speed  = SPI_SPEED_10MHZ,
        .cs_pin     = GPIO_PIN(1, 13),
        .int_pin    = GPIO_PIN(2, 29),
        .rst_pin    = GPIO_PIN(0, 11)
    },
    {
        .base.root  = &abcdev_root,
        DEV_NAME("ETH1")
        .spi        = SPI_DEV(0),
        .spi_speed  = SPI_SPEED_10MHZ,
        .cs_pin     = GPIO_PIN(0, 1),
        .int_pin    = GPIO_PIN(0, 2),
        .rst_pin    = GPIO_PIN(0, 3)
    }
};


DEV_DEF const abcdev_params_t devs[] = {
    {
        .base.root  = &abcdev_root,
        DEV_NAME("Some device")
        .spi        = SPI_DEV(0),
        .cs         = GPIO_PIN(1, 13),
        .mode       = ABCDEV_MODE_SOMETHING
    },
    {
        .base.root = &abcdev_root,
        DEV_NAME("Some other ABCDEV device")
        .spi        = SPI_DEV(0),
        .cs         = GPIO_PIN(0, 23),
        .mode       = ABCDEV_MODE_OFF
    }
};
