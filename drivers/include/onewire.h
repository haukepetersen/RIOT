

#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <stdint.h>
#include "string.h"

#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   String length of a hex encoded 1-wire ROM address (including \0)
 */
#define ONEWIRE_ROM_STR_LEN     (17U)

/**
 * @brief   Return codes used by the 1-wire driver
 */
enum {
    ONEWIRE_OK          =  0,   /**< everything fine */
    ONEWIRE_NODEV       = -1,   /**< no device connected to the bus */
    ONEWIRE_ERR_PINCFG  = -2,   /**< unable to initialize pin as open-drain */
    ONEWIRE_ERR_ROMSTR  = -3    /**< invalid ROM string given */
};

/**
 * @brief   1-Wire ROM commands
 */
enum {
    ONEWIRE_ROM_SEARCH  = 0xf0, /**< search for devices */
    ONEWIRE_ROM_READ    = 0x33, /**< read ROM code in single device config */
    ONEWIRE_ROM_MATCH   = 0x55, /**< address a specific slave */
    ONEWIRE_ROM_SKIP    = 0xcc, /**< broadcast commands to all connected devs */
    ONEWIRE_ROM_ALARM   = 0xec  /**< search for devices with active alarm */
};

typedef struct {
    gpio_t pin;
    gpio_mode_t pin_mode;
} onewire_params_t;

typedef struct {
    gpio_t pin;
    gpio_mode_t omode;
    gpio_mode_t imode;
} onewire_t;

typedef union {
    uint64_t u64;
    uint32_t u32[2];
    uint8_t u8[8];
} onewire_rom_t;


/**
 * @brief   Initialize a 1-wire bus on the given GPIO pin
 *
 * This tries to configure the pin as open-drain output and will fail, if the
 * platform does not support this GPIO mode.
 *
 * @param[in] pin   GPIO pin the bus is connected to
 *
 * @return          ONEWIRE_OK on success
 * @return          ONEWIRE_ERR_PINCFG if open-drain mode unsupported
 */
int onewire_init(onewire_t *owi, const onewire_params_t *params);

/**
 * @brief   Generate a bus reset sequence and look for connected devices
 *
 * Every bus transaction must be started with this reset sequence. During this
 * sequence it is detected, if there are any slaves connected to the bus.
 *
 * @param[in] pin   GPIO pin the bus is connected to
 *
 * @return          ONEWIRE_OK if slave(s) were found
 * @return          ONEWIRE_NODEV if no slave answered the reset sequence
 */
int onewire_reset(const onewire_t *owi, const onewire_rom_t *rom);

/**
 * @brief   Read data from the bus
 *
 * @param[in]  pin  GPIO pin the bus in connected to
 * @param[out] data buffer to write received bytes into
 * @param[in]  len  number of bytes to read from the bus
 */
void onewire_read(const onewire_t *owi, uint8_t *data, size_t len);

/**
 * @brief   Write data to the bus
 *
 * @param[in] pin   GPIO pin the bus is connected to
 * @param[in] data  buffer holding the data that is written to the bus
 * @param[in] len   number of bytes to write to the bus
 */
void onewire_write(const onewire_t *owi, const uint8_t *data, size_t len);

/**
 * @brief   Convenience function for writing a single byte to the bus
 *
 * @param[in] pin   GPIO pin the bus is connected to
 * @param[in] data  data byte to write to the bus
 */
static inline void onewire_write_byte(const onewire_t *owi, uint8_t data)
{
    onewire_write(owi, &data, 1);
}

uint8_t onewire_crc8(const uint8_t *data, size_t len);

int onewire_search(const onewire_t *owi, onewire_rom_t *rom, int *state);
// int onewire_search_next(const onewire_t *owi, onewire_rom_t *rom);


int onewire_rom_from_str(onewire_rom_t *rom, const char *str);

void onewire_rom_to_str(char *str, const onewire_rom_t *rom);

void onewire_rom_print(const onewire_rom_t *rom);

#ifdef __cplusplus
}
#endif

#endif /* ONEWIRE_H */
/** @} */
