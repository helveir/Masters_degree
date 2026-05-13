#ifndef APP_APP_H
#define APP_APP_H

#include "../control/PressureController.h"
#include "../control/SafetyManager.h"
#include "../domain/SystemContext.h"
#include "../include/config.h"
#include "../inputs/Buttons.h"
#include "../inputs/PressureSensor.h"
#include "../outputs/Buzzer.h"
#include "../outputs/Display.h"
#include "../outputs/MotorDriver.h"
#include "../outputs/RGB_LED.h"
#include "../outputs/Relay.h"

class App {
  public:
    void begin();
    void update();

  private:
    Buttons _buttons;
    Buzzer _buzzer;
    RGB_LED _rgb;
    Relay _relay;
    MotorDriver _motor;
    Display _display;
    PressureSensor _pressureSensor;
    SystemContext _context;
    bool _completionNotified = false;

    void configureDefaultMode();
    void handleButtons(unsigned long nowMs);
    void updatePressure(unsigned long nowMs);
    void updateStateMachine(unsigned long nowMs);
    int currentPhaseTargetKpa() const;
    void applyActuators(const ActuatorState& actuators);
    void render();
    void onStateChanged(SystemState previous, SystemState next);
};

#endif
