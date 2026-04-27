#ifndef CONTROL_SAFETY_MANAGER_H
#define CONTROL_SAFETY_MANAGER_H

#include "../domain/ModeConfig.h"
#include "../domain/SystemTypes.h"

struct SafetyInput {
    PressureSample sample = {};
    ModeConfig mode = {};
    SystemState state = SystemState::Idle;
    unsigned long nowMs = 0;
    unsigned long regulationStartedAtMs = 0;
};

class SafetyManager {
  public:
    static FaultCode evaluate(const SafetyInput& input) {
        if (!input.sample.valid) {
            return FaultCode::SensorFault;
        }

        const int minAllowed = input.mode.minTargetKpa - input.mode.alarmMarginKpa;
        const int maxAllowed = input.mode.maxTargetKpa + input.mode.alarmMarginKpa;

        if (input.sample.pressureKpa < minAllowed ||
            input.sample.pressureKpa > maxAllowed) {
            return FaultCode::PressureOutOfRange;
        }

        if (input.state == SystemState::Running &&
            input.nowMs - input.regulationStartedAtMs > input.mode.maxRegulationTimeMs) {
            return FaultCode::RegulationTimeout;
        }

        return FaultCode::None;
    }
};

#endif
