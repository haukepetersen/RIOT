/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     driver_sensif
 * @{
 *
 * @file
 * @brief       Generic sensor/actuator data handling
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>

#include "phydat.h"

void phydat_dump(phydat_t *data, uint8_t dim)
{
    if (data == NULL || dim > PHYDAT_DIM) {
        puts("Unable to display data object");
        return;
    }
    printf("Data:");
    for (uint8_t i = 0; i < dim; i++) {
        char tmp[PHYDAT_SCALE_STR_MAXLEN];
        phydat_scale_to_str(data->scale, tmp);
        printf("\t[%i] %i%s%s\n", (int)i, (int)data->val[i], tmp,
               phydat_unit_to_str(data->unit));
    }
}

const char *phydat_unit_to_str(uint8_t unit)
{
    switch (unit) {
        case UNIT_DEG_C:    return "°C";
        case UNIT_DEG_F:    return "°F";
        case UNIT_DEG_K:    return "K";
        case UNIT_M:        return "m";
        case UNIT_G:        return "g";
        case UNIT_DPS:      return "dps";
        case UNIT_GR:       return "G";
        case UNIT_A:        return "A";
        case UNIT_V:        return "V";
        case UNIT_GS:       return "Gs";
        case UNIT_BAR:      return "Bar";
        case UNIT_PA:       return "Pa";
        case UNIT_CD:       return "cd";
        default:            return "";
    }
}

void phydat_scale_to_str(int8_t scale, char *str)
{
    switch (scale) {
        case 0:     str[0] = '\0'; return;
        case -3:    str[0] = 'm'; break;
        case -6:    str[0] = 'u'; break;
        case -9:    str[0] = 'n'; break;
        case -12:   str[0] = 'p'; break;
        case -15:   str[0] = 'f'; break;
        case 3:     str[0] = 'k'; break;
        case 6:     str[0] = 'M'; break;
        case 9:     str[0] = 'G'; break;
        case 12:    str[0] = 'T'; break;
        case 15:    str[0] = 'P'; break;
        default:
            snprintf(str, PHYDAT_SCALE_STR_MAXLEN, "E%i", (int)scale);
            return;
    }
    str[1] = '\0';
}
