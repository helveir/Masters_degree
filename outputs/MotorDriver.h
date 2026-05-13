#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>

class MotorDriver {
  public:
    void begin(uint8_t in1Pin, uint8_t in2Pin);
    void forwardOn();
    void stop();
    bool isRunning() const;

  private:
    uint8_t _in1Pin = 0;
    uint8_t _in2Pin = 0;
    bool _running = false;
};

#endif
