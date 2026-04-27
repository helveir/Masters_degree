#ifndef DOMAIN_SYSTEM_CONTEXT_H
#define DOMAIN_SYSTEM_CONTEXT_H

#include "ModeConfig.h"
#include "SystemTypes.h"

struct SystemContext {
    SystemState state = SystemState::Idle;
    FaultCode fault = FaultCode::None;
    ModeConfig mode = {};
    PressureSample pressure = {};
    ActuatorState actuators = {};
    int targetPressureKpa = 0;
    unsigned long stateStartedAtMs = 0;
    unsigned long regulationStartedAtMs = 0;
    unsigned long holdStartedAtMs = 0;

    void setState(SystemState nextState, unsigned long nowMs) {
        state = nextState;
        stateStartedAtMs = nowMs;
    }

    void resetOutputs() {
        actuators.feedOn = false;
        actuators.suctionOn = false;
    }

    void resetFault() {
        fault = FaultCode::None;
    }

    void startRun(unsigned long nowMs) {
        resetFault();
        resetOutputs();
        setState(SystemState::Running, nowMs);
        regulationStartedAtMs = nowMs;
        holdStartedAtMs = 0;
    }

    void stop(unsigned long nowMs) {
        resetOutputs();
        setState(SystemState::Idle, nowMs);
        holdStartedAtMs = 0;
        regulationStartedAtMs = 0;
    }

    void startHolding(unsigned long nowMs) {
        resetOutputs();
        setState(SystemState::Holding, nowMs);
        holdStartedAtMs = nowMs;
    }

    void complete(unsigned long nowMs) {
        resetOutputs();
        setState(SystemState::Completed, nowMs);
    }

    void trip(FaultCode code, unsigned long nowMs) {
        fault = code;
        resetOutputs();
        setState(SystemState::Alarm, nowMs);
    }
};

#endif
