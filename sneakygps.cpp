

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

#include "sneakygps.h"

uint8_t UtmCalculator::getUTMLetter(const int8_t lat) {
    if (lat < -80 || lat >= 80) {
        return 'Z';
    }
    int8_t base = (lat + 80) / 8;
    return base + ((base >= 11) ? 'E' : (base >= 6) ? 'D' : 'C');
}

UtmCalculator::UtmCalculator(const float lat, const float lon) {
    uint8_t zoneNumber =  findZone(lat, lon);
    convertLatLon(lat, lon, zoneNumber);
}

uint8_t UtmCalculator::findZone(const float lat, const float lon) {
    uint8_t zoneNumber = int((lon + 180) / 6) + 1;
    if (lat >= 56.0 && lat < 64.0 && lon >= 3.0 && lon < 12.0) {
        zoneNumber = 32;
    }
    if (lat >= 72.0 && lat < 84.0) { // Svalbard
        if (lon >= 0.0 && lon < 9.0) zoneNumber = 31;
        else if (lon >= 9.0 && lon < 21.0) zoneNumber = 33;
        else if (lon >= 21.0 && lon < 33.0) zoneNumber = 35;
        else if (lon >= 33.0 && lon < 42.0) zoneNumber = 37;
    }
    sprintf((char *) this->utmZone, "%d%c", zoneNumber, getUTMLetter((uint8_t) lat));
    return zoneNumber;
}

static const float k0 = 0.9996;
static const float pi = 3.14159265;
static const float deg2rad = pi / 180;
static const float eccSquared = 0.00669438;
static const float eccPrimeSquared = eccSquared/(1-eccSquared);
static const float earthRadius = 6378137;

void UtmCalculator::convertLatLon(const float lat, const float lon, const uint8_t zoneNumber) {
    float lonOrigin = (zoneNumber - 1)*6 - 180 + 3;  // +3 puts origin in middle of zone
    float lonOriginRad = lonOrigin * deg2rad;
    float latRad = lat*deg2rad;
    float lonRad = lon*deg2rad;
    float n = earthRadius/sqrt(1-eccSquared*sin(latRad)*sin(latRad));
    float t = tan(latRad)*tan(latRad);
    float c = eccPrimeSquared*cos(latRad)*cos(latRad);
    float a = cos(latRad)*(lonRad-lonOriginRad);
    float m = earthRadius*((1 - eccSquared/4
        - 3*eccSquared*eccSquared/64
        - 5*eccSquared*eccSquared*eccSquared/256)*latRad
        - (3*eccSquared/8 + 3*eccSquared*eccSquared/32
        + 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*latRad)
        + (15*eccSquared*eccSquared/256
        + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*latRad)
        - (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*latRad));
    this->utmEasting = (float)(k0*n*(a+(1-t+c)*a*a*a/6
        + (5-18*t+t*t+72*c-58*eccPrimeSquared)*a*a*a*a*a/120) + 500000.0);
    this->utmNorthing = (float)(k0*(m+n*tan(latRad)*(a*a/2+(5-t+9*c+4*c*c)*a*a*a*a/24
        + (61-58*t+t*t+600*c-330*eccPrimeSquared)*a*a*a*a*a*a/720)));
    if (lat < 0) {
        this->utmNorthing += 10000000.0;
    }
}


