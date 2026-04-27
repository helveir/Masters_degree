#ifndef INPUTS_PRESSURE_SENSOR_H
#define INPUTS_PRESSURE_SENSOR_H

#include <Arduino.h>
#include "../domain/SystemTypes.h"

class PressureSensor {
  public:
    void begin(uint8_t pin, bool simulationEnabled);
    PressureSample read();
    void updateSimulation(bool feedOn, bool suctionOn, unsigned long nowMs);
    void setSimulatedPressure(int pressureKpa);

  private:
    uint8_t _pin = A0;
    bool _simulationEnabled = false;
    int _simulatedPressureKpa = 0;
    unsigned long _lastSimulationStepMs = 0;
};

#endif
