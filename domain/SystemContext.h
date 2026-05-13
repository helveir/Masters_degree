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
    int pressureRangeKpa = 0;
    unsigned long stateStartedAtMs = 0;

    void setState(SystemState nextState, unsigned long nowMs) {
        state = nextState;
        stateStartedAtMs = nowMs;
    }

    void resetOutputs() {
        actuators.feedOn = false;
        actuators.suctionOn = false;
        actuators.motorOn = false;
    }

    void resetFault() {
        fault = FaultCode::None;
    }

    void startPressurizing(unsigned long nowMs) {
        resetFault();
        resetOutputs();
        setState(SystemState::Pressurizing, nowMs);
    }

    void startDepressurizing(unsigned long nowMs) {
        resetOutputs();
        setState(SystemState::Depressurizing, nowMs);
    }

    void startReturningToZero(unsigned long nowMs) {
        resetOutputs();
        setState(SystemState::ReturningToZero, nowMs);
    }

    void stop(unsigned long nowMs) {
        resetOutputs();
        setState(SystemState::Idle, nowMs);
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
