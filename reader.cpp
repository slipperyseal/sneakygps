
#include "reader.h"

UartRead::UartRead() {
    this->rxDataSig = me.allocateSignal();
    this->usartRx = new UsartRx(0);
    this->usartRx->setCommsParams(9600);
    this->usartRx->enable(256, rxDataSig, 0UL);
    this->available = 0;
    this->position = 0;
}

void UartRead::waitForData() {
    auto wokeSigs = me.wait(rxDataSig);
    if (wokeSigs & rxDataSig) {
        this->rxBuffer = this->usartRx->getCurrentBuffer(this->available);
    }
}

uint8_t UartRead::read() {
    while (position >= available) {
        position = 0;
        waitForData();
    }
    return this->rxBuffer[this->position++];
}

void UartRead::flush() {
    this->usartRx->flush();
}

void UartRead::seekToValue(uint8_t until) {
    while (this->read() != until);
};

uint8_t UartRead::readUntil(uint8_t * buffer, uint8_t size, uint8_t until) {
    uint8_t x=0;
    uint8_t c;
    while ((c = this->read()) != until && x < size) {
        buffer[x++] = c;
    }
    buffer[x++] = 0;
    return x;
};

