//
// SneakyGPS
//

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <avr/interrupt.h>
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

#include "sevenseg.h"
#include "reader.h"
#include "sneakygps.h"

using namespace zero;

SevenSegment sevenSegment = SevenSegment();

uint8_t * findDecimal(uint8_t *string) {
    int c;
    while ((c = *string++) != 0 && c != '.');
    return string;
}

uint32_t parseInt(uint8_t * string) {
    uint8_t c;
    uint32_t value = 0;
    while ((c = *string++) != 0 && c != '.') {
        value *= 10;
        value += (c-'0');
    }
    return value;
}

float convertNMEA(uint8_t * value, uint8_t * direction) {
    uint16_t left = parseInt(value);
    uint16_t deg = left/100;
    uint16_t hours = left-(deg*100);
    uint8_t * dec = findDecimal(value);

    uint8_t remain[8];
    remain[0] = (hours/10) + '0';
    remain[1] = hours - ((hours/10)*10) + '0';
    remain[2] = '.';
    remain[3] = dec[0];
    remain[4] = dec[1];
    remain[5] = dec[2];
    remain[6] = dec[3];
    remain[7] = 0;

    float result = deg + ((float)atof((char*)remain)/60.0);
    if (direction[0] == 'S' ||  direction[0] == 'W') {
        result = 0-result;
    }
    return result;
}

void setLatLonString(uint8_t * string, float value) {
    sprintf((char*)string, "%f", (double)value);
    findDecimal(string)[4] = 0;
}

bool isGPGGA(uint8_t * string) {
    return string[0] == 'G' && string[1] == 'P' && string[2] == 'G' && string[3] == 'G' && string[4] == 'A';
}

void printUtmLatLon(const float latf, const float lonf) {
    UtmCalculator utmCalculator = UtmCalculator(latf, lonf);

    sevenSegment.writeStringBrisk((uint8_t*)" UTM ");
    sevenSegment.writeStringSlow(utmCalculator.utmZone);

    uint8_t string[20];
    sprintf((char*)string, (char*)" E %lu", (unsigned long)utmCalculator.utmEasting);
    sevenSegment.writeStringSlow(string);

    sprintf((char*)string, (char*)" N %lu", (unsigned long)utmCalculator.utmNorthing);
    sevenSegment.writeStringSlow(string);

    sevenSegment.writeStringBrisk((uint8_t*)" LAT ");
    setLatLonString((uint8_t*)string, latf);
    sevenSegment.writeStringSlow(string);

    sevenSegment.writeStringBrisk((uint8_t*)" LON ");
    setLatLonString((uint8_t*)string, lonf);
    sevenSegment.writeStringSlow(string);

    sevenSegment.writeStringSlow((uint8_t*)"  ");
}

void processGPGGA(UartRead * uartRead) {
    uint8_t data1[16], data2[16];

    uartRead->flush();
    uartRead->seekToValue('$');
    uartRead->readUntil(data1, sizeof(data1), ',');
    if (isGPGGA(data1)) {
        uartRead->seekToValue(',');
        uartRead->readUntil(data1, sizeof(data1), ',');
        uartRead->readUntil(data2, sizeof(data2), ',');

        if (data2[0] != 0) {
            float lat = convertNMEA(data1, data2);
            uartRead->readUntil(data1, sizeof(data1), ',');
            uartRead->readUntil(data2, sizeof(data2), ',');
            if (data2[0] != 0) {
                printUtmLatLon(lat, convertNMEA(data1, data2) );
            }
        }
        sevenSegment.blip();
    }
}

int sneakyGpsThread() {
    sevenSegment.blip();
    sevenSegment.writeStringBrisk((uint8_t*)"SNEAKY GPS");

    UartRead uartRead = UartRead();
    for (;;) {
        processGPGGA(&uartRead);
    }
}

int idleThreadEntry() {
    for (;;) { }
}

void startup_sequence() {
    DDRB |= 0b00000011;
    DDRD |= 0b11111100;
    new Thread(256, sneakyGpsThread, TF_FIRE_AND_FORGET);
}
