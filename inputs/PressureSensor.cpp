#include "PressureSensor.h"

#include "../include/config.h"

void PressureSensor::begin(uint8_t pin, bool simulationEnabled) {
    _pin = pin;
    _simulationEnabled = simulationEnabled;
    _simulatedPressureKpa = DEFAULT_TARGET_PRESSURE_KPA;
    _lastSimulationStepMs = 0;

    if (!_simulationEnabled) {
        pinMode(_pin, INPUT);
    }
}

PressureSample PressureSensor::read() {
    if (_simulationEnabled) {
        PressureSample sample;
        sample.pressureKpa = _simulatedPressureKpa;
        sample.valid = true;
        return sample;
    }

    const int raw = analogRead(_pin);
    const long mapped = map(raw,
                            PRESSURE_SENSOR_RAW_MIN,
                            PRESSURE_SENSOR_RAW_MAX,
                            PRESSURE_SENSOR_KPA_MIN,
                            PRESSURE_SENSOR_KPA_MAX);

    PressureSample sample;
    sample.pressureKpa = static_cast<int>(mapped);
    sample.valid = true;
    return sample;
}

void PressureSensor::updateSimulation(bool feedOn, bool suctionOn, unsigned long nowMs) {
    if (!_simulationEnabled) {
        return;
    }

    if (_lastSimulationStepMs == 0) {
        _lastSimulationStepMs = nowMs;
        return;
    }

    if (nowMs - _lastSimulationStepMs < PRESSURE_SIMULATION_STEP_MS) {
        return;
    }

    _lastSimulationStepMs = nowMs;

    if (feedOn && !suctionOn) {
        _simulatedPressureKpa++;
    } else if (suctionOn && !feedOn) {
        _simulatedPressureKpa--;
    }

    if (_simulatedPressureKpa < PRESSURE_SENSOR_KPA_MIN) {
        _simulatedPressureKpa = PRESSURE_SENSOR_KPA_MIN;
    }

    if (_simulatedPressureKpa > PRESSURE_SENSOR_KPA_MAX) {
        _simulatedPressureKpa = PRESSURE_SENSOR_KPA_MAX;
    }
}

void PressureSensor::setSimulatedPressure(int pressureKpa) {
    _simulatedPressureKpa = pressureKpa;
}
