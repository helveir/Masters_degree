#include "MotorDriver.h"

void MotorDriver::begin(uint8_t in1Pin, uint8_t in2Pin) {
    _in1Pin = in1Pin;
    _in2Pin = in2Pin;

    pinMode(_in1Pin, OUTPUT);
    pinMode(_in2Pin, OUTPUT);
    stop();
}

void MotorDriver::forwardOn() {
    digitalWrite(_in1Pin, HIGH);
    digitalWrite(_in2Pin, LOW);
    _running = true;
}

void MotorDriver::stop() {
    digitalWrite(_in1Pin, LOW);
    digitalWrite(_in2Pin, LOW);
    _running = false;
}

bool MotorDriver::isRunning() const {
    return _running;
}
