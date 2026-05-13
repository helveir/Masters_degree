#ifndef CONTROL_PRESSURE_CONTROLLER_H
#define CONTROL_PRESSURE_CONTROLLER_H

#include "../domain/SystemTypes.h"

struct PressureControlInput {
    int currentPressureKpa = 0;
    int targetPressureKpa = 0;
    int toleranceKpa = 0;
    bool depressurizing = false;
};

struct PressureControlResult {
    ActuatorState actuators = {};
    bool targetReached = false;
};

class PressureController {
  public:
    static PressureControlResult compute(const PressureControlInput& input) {
        PressureControlResult result;

        if (!input.depressurizing) {
            if (input.currentPressureKpa < input.targetPressureKpa - input.toleranceKpa) {
                result.actuators.feedOn = true;
                result.actuators.suctionOn = false;
                result.actuators.motorOn = true;
                return result;
            }

            result.targetReached = true;
            return result;
        }

        if (input.currentPressureKpa > input.targetPressureKpa + input.toleranceKpa) {
            result.actuators.feedOn = false;
            result.actuators.suctionOn = true;
            result.actuators.motorOn = true;
            return result;
        }

        result.targetReached = true;
        return result;
    }
};

#endif
