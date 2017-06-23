

#ifndef SPFD5408_PARAMS_H
#define SPFD5408_PARAMS_H

#include "arduino_pinmap.h"

static const spfd5408_params_t spfd5408_params[] = {
    {
        .width  = 320,
        .height = 240,
        .rst    = ARDUINO_PIN_A4,
        .cs     = ARDUINO_PIN_A3,
        .rs     = ARDUINO_PIN_A2,
        .wr     = ARDUINO_PIN_A1,
        .rd     = ARDUINO_PIN_A0,
        .dat    = { ARDUINO_PIN_8, ARDUINO_PIN_9, ARDUINO_PIN_2, ARDUINO_PIN_3,
                    ARDUINO_PIN_4, ARDUINO_PIN_5, ARDUINO_PIN_6, ARDUINO_PIN_7 }
    }
}

#endif /* SPFD5408_PARAMS_H */
/** @} */
