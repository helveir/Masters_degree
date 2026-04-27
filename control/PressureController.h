#ifndef CONTROL_PRESSURE_CONTROLLER_H
#define CONTROL_PRESSURE_CONTROLLER_H

#include "../domain/SystemTypes.h"

struct PressureControlInput {
    int currentPressureKpa = 0;
    int targetPressureKpa = 0;
    int toleranceKpa = 0;
    unsigned long nowMs = 0;
    unsigned long holdStartedAtMs = 0;
    unsigned long holdDurationMs = 0;
    bool holdStateActive = false;
};

struct PressureControlResult {
    ActuatorState actuators = {};
    bool pressureStable = false;
    bool holdComplete = false;
};

class PressureController {
  public:
    static PressureControlResult compute(const PressureControlInput& input) {
        PressureControlResult result;

        if (input.currentPressureKpa < input.targetPressureKpa - input.toleranceKpa) {
            result.actuators.feedOn = true;
            result.actuators.suctionOn = false;
            return result;
        }

        if (input.currentPressureKpa > input.targetPressureKpa + input.toleranceKpa) {
            result.actuators.feedOn = false;
            result.actuators.suctionOn = true;
            return result;
        }

        result.pressureStable = true;
        if (input.holdStateActive &&
            input.nowMs - input.holdStartedAtMs >= input.holdDurationMs) {
            result.holdComplete = true;
        }

        return result;
    }
};

#endif
