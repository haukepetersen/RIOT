/*
 * Copyright (C) 2017 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_firmware
 * @{
 *
 * @file
 * @brief       Firmware update helper functions
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */

#include <string.h>

#include "firmware_update.h"
#include "log.h"
#include "ota_pubkey.h"

static inline size_t min(size_t a, size_t b)
{
    return a <= b ? a : b;
}

int firmware_update_init(firmware_update_t *firmware_update, int target_slot)
{
    memset(firmware_update, 0, sizeof(firmware_update_t));
    sha256_init(&firmware_update->sha256);

    firmware_update->target_slot = target_slot;
    firmware_update->flashpage = flashpage_page(firmware_get_metadata(target_slot));

    return 0;
}

int firmware_update_putbytes(firmware_update_t *state, size_t offset, const uint8_t *bytes, size_t len)
{
    firmware_metadata_t *metadata = (firmware_metadata_t *)state->metadata_buf;

    while(len) {
        LOG_DEBUG("%s: processing the next %u bytes (offset=%u)\n", __func__, len, state->offset);
        size_t flashpage_pos = state->offset % FLASHPAGE_SIZE;
        size_t flashpage_avail = FLASHPAGE_SIZE - flashpage_pos;

        size_t to_copy = min(flashpage_avail, len);
        memcpy(state->flashpage_buf + flashpage_pos, bytes, to_copy);
        flashpage_avail -= to_copy;

        state->offset += to_copy;
        flashpage_pos += to_copy;
        bytes += to_copy;
        len -= to_copy;

        if (!flashpage_avail || (state->offset == (metadata->size + sizeof(firmware_metadata_t)))) {
            if (state->flashpage == flashpage_page(firmware_get_metadata(state->target_slot))) {
                memcpy(state->metadata_buf, state->flashpage_buf, sizeof(state->flashpage_buf));
                LOG_INFO("%s: verifying metadata\n", __func__);
                /* check metadata magic nr, checksum and signature */
                if (firmware_validate_metadata_signature(metadata, ota_public_key)) {
                    LOG_WARNING("[update] Cannot verify firmware signature. Aborting.\n");
                    return -1;
                }
                else {
                    LOG_INFO("[update] Signature check successful!\n");
                }
                if (metadata->start_addr != firmware_get_image_startaddr(state->target_slot)) {
                    LOG_WARNING("[update] Image start address doesn't match selected slot. Aborting.\n");
                    return -1;
                }

                if (!flashpage_avail) {
                    if (flashpage_write_and_verify(state->flashpage, state->flashpage_buf + sizeof(firmware_metadata_t)) != FLASHPAGE_OK) {
                        LOG_WARNING("%s: verify failed\n", __func__);
                        return -1;
                    }
                    /* Perform sha256 of the bytes *after* the metadata */
                    LOG_INFO("%s: hashing %u bytes\n", __func__, flashpage_pos - sizeof(firmware_metadata_t));
                    sha256_update(&state->sha256, state->flashpage_buf + sizeof(firmware_metadata_t), flashpage_pos - sizeof(firmware_metadata_t));
                }
                else {
                    flashpage_write(state->flashpage, NULL);
                }
            }
            else {
                LOG_INFO("%s: writing to flashpage %u\n", __func__, state->flashpage);
                if (flashpage_write_and_verify(state->flashpage, state->flashpage_buf) != FLASHPAGE_OK) {
                    LOG_WARNING("%s: verify failed\n", __func__);
                    return -1;
                }
                LOG_INFO("%s: hashing %u bytes\n", __func__, flashpage_pos);
                sha256_update(&state->sha256, state->flashpage_buf, flashpage_pos);
                LOG_INFO("%s: hashing %u bytes done\n", __func__, flashpage_pos);
            }
            state->flashpage++;
        }
    }

    return 0;
}

int firmware_update_finish(firmware_update_t *state)
{
    /* re-purposing flashpage_buf here, its is not needed anymore */
    sha256_final(&state->sha256, state->flashpage_buf);

    LOG_INFO("%s: finalizing image hash...\n", __func__);
    firmware_metadata_t *metadata = (firmware_metadata_t *) state->metadata_buf;
    if (memcmp(state->flashpage_buf, metadata->hash, SHA256_DIGEST_LENGTH)) {
        LOG_WARNING("%s: image hash incorrect\n", __func__);
        return -1;
    }
    else {
        LOG_INFO("%s: hash verified, flashing metadata block...\n", __func__);
        int flashpage = flashpage_page(firmware_get_metadata(state->target_slot));
        LOG_INFO("%s: writing to flashpage %u\n", __func__, flashpage);
        if (flashpage_write_and_verify(flashpage, state->metadata_buf) != FLASHPAGE_OK) {
            LOG_WARNING("%s: verify failed\n", __func__);
            return -1;
        }
    }

    LOG_INFO("%s: firmware flashing completed successfully\n", __func__);

    return 0;
}
