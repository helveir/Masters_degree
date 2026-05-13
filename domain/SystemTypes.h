#ifndef DOMAIN_SYSTEM_TYPES_H
#define DOMAIN_SYSTEM_TYPES_H

#include <stdint.h>

enum class SystemState : uint8_t {
    Idle = 0,
    Pressurizing = 1,
    Alarm = 2,
    Depressurizing = 3,
    ReturningToZero = 4,
    Completed = 5
};

enum class FaultCode : uint8_t {
    None = 0,
    SensorFault,
    PressureOutOfRange,
    RegulationTimeout
};

struct PressureSample {
    int pressureKpa = 0;
    bool valid = false;
};

struct ActuatorState {
    bool feedOn = false;
    bool suctionOn = false;
    bool motorOn = false;
};

#endif
