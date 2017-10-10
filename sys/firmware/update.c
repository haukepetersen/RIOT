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
#include "od.h"

static inline size_t min(size_t a, size_t b)
{
    return a <= b ? a : b;
}

int firmware_update_init(firmware_update_t *firmware_update, int target_slot)
{
    LOG_INFO("%s: target slot %i\n", __func__, target_slot);
    memset(firmware_update, 0, sizeof(firmware_update_t));

    firmware_update->target_slot = target_slot;
    firmware_update->flashpage = flashpage_page(firmware_get_metadata(target_slot));
    firmware_update->state = FIRMWARE_UPDATE_INITIALIZED;

    return 0;
}

int firmware_update_putbytes(firmware_update_t *state, size_t offset, const uint8_t *bytes, size_t len)
{
    firmware_metadata_t *metadata = &state->metadata_buf;

    LOG_INFO("%s: processing the next %u bytes (offset=%u)\n", __func__, len, state->offset);

    while(len) {
        size_t flashpage_pos = state->offset % FLASHPAGE_SIZE;
        size_t flashpage_avail = FLASHPAGE_SIZE - flashpage_pos;

        size_t left = 1;

        if (state->state == FIRMWARE_UPDATE_VERIFIED) {
            left = (metadata->size + sizeof(firmware_metadata_t) - offset);
            len = min(len, left);
        }

        size_t to_copy = min(flashpage_avail, len);

        memcpy(state->flashpage_buf + flashpage_pos, bytes, to_copy);
        flashpage_avail -= to_copy;

        state->offset += to_copy;
        flashpage_pos += to_copy;
        bytes += to_copy;
        len -= to_copy;
        left -= to_copy;

        if ((state->state != FIRMWARE_UPDATE_VERIFIED) && ((offset + to_copy) >= sizeof(firmware_metadata_t))) {
            /* copy metadata from flashpage buffer to metadata buffer */
            memcpy(metadata, state->flashpage_buf, sizeof(firmware_metadata_t));

            LOG_INFO("%s: verifying metadata\n", __func__);
            if (metadata->start_addr != firmware_get_image_startaddr(state->target_slot)) {
                LOG_WARNING("[update] Image start address doesn't match selected slot. Aborting.\n");
                LOG_WARNING("[update] (image start=%p slot start=%p)\n", (void*)metadata->start_addr, \
                        (void*)firmware_get_image_startaddr(state->target_slot));
                return -1;
            }

            /* check metadata magic nr, checksum and signature */
            if (firmware_validate_metadata_signature(metadata, ota_public_key)) {
                LOG_WARNING("[update] Cannot verify firmware signature. Aborting.\n");
                return -1;
            }
            else {
                state->state = FIRMWARE_UPDATE_VERIFIED;
                LOG_INFO("[update] Signature check successful!\n");
            }

            /* clear metadata from flashpage */
            memset(state->flashpage_buf, '\0', sizeof(firmware_metadata_t));
        }

        if ((!flashpage_avail) || (!left)) {
            LOG_DEBUG("%s: writing to flashpage %u\n", __func__, state->flashpage);
            if (flashpage_write_and_verify(state->flashpage, state->flashpage_buf) != FLASHPAGE_OK) {
                LOG_WARNING("%s: verify failed\n", __func__);
                return -1;
            }
            state->flashpage++;
        }
    }

    return 0;
}

int firmware_update_finish(firmware_update_t *state)
{
    int res = -1;

    void *slot_start = firmware_get_metadata(state->target_slot);
    firmware_metadata_t *metadata = &state->metadata_buf;

    LOG_INFO("%s: calculating image hash...\n", __func__);
    sha256_init(&state->sha256);
    sha256_update(&state->sha256, slot_start + sizeof(firmware_metadata_t), metadata->size);

    sha256_final(&state->sha256, state->flashpage_buf);

    if (memcmp(state->flashpage_buf, metadata->hash, SHA256_DIGEST_LENGTH)) {
        LOG_WARNING("%s: image hash incorrect\n", __func__);
        goto out;
    }

    LOG_INFO("%s: hash verified, re-flashing first block...\n", __func__);
    void *firstpage;
    if (sizeof(firmware_metadata_t) < FLASHPAGE_SIZE) {
        firstpage = state->flashpage_buf;
        memcpy(firstpage, metadata, sizeof(firmware_metadata_t));
        memcpy(firstpage + sizeof(firmware_metadata_t), \
                slot_start + sizeof(firmware_metadata_t), \
                FLASHPAGE_SIZE - sizeof(firmware_metadata_t));
    }
    else {
        firstpage = metadata;
    }

    int flashpage = flashpage_page(slot_start);
    LOG_INFO("%s: writing to flashpage %u\n", __func__, flashpage);
    if (flashpage_write_and_verify(flashpage, firstpage) != FLASHPAGE_OK) {
        LOG_WARNING("%s: verify failed\n", __func__);
        goto out;
    }

    LOG_INFO("%s: firmware flashing completed successfully\n", __func__);
    res = 0;

out:
    state->state = FIRMWARE_UPDATE_IDLE;
    return res;
}
