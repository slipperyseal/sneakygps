
#ifndef _H_READER_H
#define _H_READER_H

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

class UartRead {
public:
    UartRead();

    uint8_t read();
    void seekToValue(uint8_t until);
    uint8_t readUntil(uint8_t * buffer, uint8_t size, uint8_t until);
    void flush();

private:
    SignalField rxDataSig;
    UsartRx * usartRx;
    uint8_t * rxBuffer;
    uint16_t available;
    uint16_t position;

    void waitForData();
};

#endif