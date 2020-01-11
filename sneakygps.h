
#ifndef _H_SNEAKY_H
#define _H_SNEAKY_H

#include <stdint.h>
#include <string.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "thread.h"
#include "memory.h"
#include "debug.h"
#include "drivers/usart.h"
#include "drivers/suart.h"
#include "util.h"
#include "drivers/sram.h"

using namespace zero;

class UtmCalculator {
public:
    float utmNorthing;
    float utmEasting;
    uint8_t utmZone[6];

    UtmCalculator(const float lat, const float lon);

private:
    uint8_t findZone(const float lat, const float lon);
    void convertLatLon(const float lat, const float lon, const uint8_t zoneNumber);
    uint8_t getUTMLetter(const int8_t lat);
};

#endif