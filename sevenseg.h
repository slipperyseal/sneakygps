
#ifndef _H_SEVENSEG
#define _H_SEVENSEG

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

class SevenSegmentFont {
private:
    static const uint8_t sevenSegmentFontBitmap[128] PROGMEM;
public:
    uint8_t getBitmap(uint8_t character);
    uint8_t getSneakyBitmap(uint8_t bitmap);
};

class SevenSegment {
private:
    SevenSegmentFont sevenSegmentFont;
public:
    SevenSegment();
    void set(uint8_t bits);
    
    void writeStringBrisk(uint8_t * string);
    void writeStringSlow(uint8_t * string);
    void blip();
};

#endif
