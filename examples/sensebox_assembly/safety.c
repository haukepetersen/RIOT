/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Safety routines to prevent overwatering.
 *
 * @author      Daniel Petry <daniel.petry@fu-berlin.de>
 *
 * @}
 */

#ifdef VALVE_WATERING_ON
bool safety_test(int sample, uint8_t valve_no)
{
    if (sample < (valves[valve_no].watering_level - SAFETY_LOWER)){
        valves[valve_no].low_disable = true;
    }

    if (sample > (valves[valve_no].watering_level)) {
        valves[valve_no].low_disable = false;
    }

    if (sample > (valves[valve_no].watering_level + SAFETY_UPPER)){
        valves[valve_no].high_disable = true;
    }

    DEBUG_PRINT("valve - %4d, watering level - %4d, sample value - %4d,",
            valve_no, valves[valve_no].watering_level, sample);
    DEBUG_PRINT("low disable - %1d, high disable - %1d\n",
            valves[valve_no].low_disable, valves[valve_no].high_disable);

    if (valves[valve_no].low_disable || valves[valve_no].high_disable) {
        return false;
    }

    return true;
}
#endif
