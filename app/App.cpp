#include "App.h"

void App::begin() {
    Serial.begin(9600);

    _buzzer.begin(BUZZER_PIN);
    _rgb.begin(RGB_RED_PIN, RGB_GREEN_PIN, RGB_BLUE_PIN);
    _buttons.begin(BTN_DECREASE_PIN, BTN_START_PIN, BTN_INCREASE_PIN);
    _relay.begin(RELAY_FEED_PIN, RELAY_SUCTION_PIN);
    _pressureSensor.begin(PRESSURE_SENSOR_PIN, PRESSURE_SENSOR_SIMULATION);

    configureDefaultMode();

    if (! _display.begin()) {
        Serial.println("Display not found");
    }

    _context.targetPressureKpa = _context.mode.defaultTargetKpa;
    _context.pressure = _pressureSensor.read();
    _context.setState(SystemState::Idle, millis());
    _rgb.setReady();
    render();
    Serial.println("System ready");
}

void App::update() {
    const unsigned long nowMs = millis();

    _buttons.update();
    _buzzer.update();

    handleButtons(nowMs);
    updatePressure(nowMs);
    updateStateMachine(nowMs);
    render();
}

void App::configureDefaultMode() {
    _context.mode.minTargetKpa = TARGET_PRESSURE_MIN_KPA;
    _context.mode.maxTargetKpa = TARGET_PRESSURE_MAX_KPA;
    _context.mode.defaultTargetKpa = DEFAULT_TARGET_PRESSURE_KPA;
    _context.mode.toleranceKpa = PRESSURE_TOLERANCE_KPA;
    _context.mode.alarmMarginKpa = PRESSURE_ALARM_MARGIN_KPA;
    _context.mode.holdDurationMs = HOLD_DURATION_MS;
    _context.mode.maxRegulationTimeMs = MAX_REGULATION_TIME_MS;
}

void App::handleButtons(unsigned long nowMs) {
    const bool allowSetpointChange =
        _context.state == SystemState::Idle || _context.state == SystemState::Completed;

    if (allowSetpointChange) {
        if (_buttons.isIncreasePressed() || _buttons.isIncreaseHeld()) {
            if (_context.targetPressureKpa < _context.mode.maxTargetKpa) {
                _context.targetPressureKpa++;
                _buzzer.beepShort();
            }
        }

        if (_buttons.isDecreasePressed() || _buttons.isDecreaseHeld()) {
            if (_context.targetPressureKpa > _context.mode.minTargetKpa) {
                _context.targetPressureKpa--;
                _buzzer.beepShort();
            }
        }
    }

    if (_buttons.isStartPressed()) {
        const SystemState previous = _context.state;

        if (_context.state == SystemState::Idle || _context.state == SystemState::Completed) {
            _context.startRun(nowMs);
            _completionNotified = false;
            _buzzer.beepShort();
        } else if (_context.state == SystemState::Alarm) {
            _context.stop(nowMs);
            _context.resetFault();
            _completionNotified = false;
            _buzzer.alarmStop();
            _buzzer.beepShort();
        } else {
            _context.stop(nowMs);
            _completionNotified = false;
            _buzzer.beepShort();
        }

        onStateChanged(previous, _context.state);
    }
}

void App::updatePressure(unsigned long nowMs) {
    _pressureSensor.updateSimulation(_context.actuators.feedOn,
                                     _context.actuators.suctionOn,
                                     nowMs);
    _context.pressure = _pressureSensor.read();
}

void App::updateStateMachine(unsigned long nowMs) {
    if (_context.state == SystemState::Idle ||
        _context.state == SystemState::Completed ||
        _context.state == SystemState::Alarm) {
        ActuatorState idleActuators;
        applyActuators(idleActuators);
        return;
    }

    SafetyInput safetyInput;
    safetyInput.sample = _context.pressure;
    safetyInput.mode = _context.mode;
    safetyInput.state = _context.state;
    safetyInput.nowMs = nowMs;
    safetyInput.regulationStartedAtMs = _context.regulationStartedAtMs;
    const FaultCode fault = SafetyManager::evaluate(safetyInput);

    if (fault != FaultCode::None) {
        const SystemState previous = _context.state;
        _context.trip(fault, nowMs);
        onStateChanged(previous, _context.state);
        ActuatorState safeActuators;
        applyActuators(safeActuators);
        return;
    }

    PressureControlInput controlInput;
    controlInput.currentPressureKpa = _context.pressure.pressureKpa;
    controlInput.targetPressureKpa = _context.targetPressureKpa;
    controlInput.toleranceKpa = _context.mode.toleranceKpa;
    controlInput.nowMs = nowMs;
    controlInput.holdStartedAtMs = _context.holdStartedAtMs;
    controlInput.holdDurationMs = _context.mode.holdDurationMs;
    controlInput.holdStateActive = _context.state == SystemState::Holding;
    const PressureControlResult result = PressureController::compute(controlInput);

    if (_context.state == SystemState::Running && result.pressureStable) {
        const SystemState previous = _context.state;
        _context.startHolding(nowMs);
        onStateChanged(previous, _context.state);
    } else if (_context.state == SystemState::Holding && result.holdComplete) {
        const SystemState previous = _context.state;
        _context.complete(nowMs);
        onStateChanged(previous, _context.state);
        ActuatorState completedActuators;
        applyActuators(completedActuators);
        return;
    }

    applyActuators(result.actuators);
}

void App::applyActuators(const ActuatorState& actuators) {
    _context.actuators = actuators;

    if (actuators.feedOn) {
        _relay.feedOn();
        _relay.suctionOff();
        return;
    }

    if (actuators.suctionOn) {
        _relay.feedOff();
        _relay.suctionOn();
        return;
    }

    _relay.bothOff();
}

void App::render() {
    switch (_context.state) {
        case SystemState::Idle:
        case SystemState::Completed:
            _rgb.setReady();
            break;
        case SystemState::Running:
        case SystemState::Holding:
            _rgb.setWorking();
            break;
        case SystemState::Alarm:
            _rgb.setAlarm();
            break;
    }

    _display.update(_context.pressure.pressureKpa,
                    _context.targetPressureKpa,
                    static_cast<int>(_context.state),
                    _context.actuators.feedOn,
                    _context.actuators.suctionOn);
}

void App::onStateChanged(SystemState previous, SystemState next) {
    if (previous == next) {
        return;
    }

    if (next == SystemState::Alarm) {
        _buzzer.alarmStart();
        Serial.println("ALARM");
        return;
    }

    if (previous == SystemState::Alarm) {
        _buzzer.alarmStop();
    }

    if (next == SystemState::Holding) {
        Serial.println("HOLD");
        return;
    }

    if (next == SystemState::Completed && !_completionNotified) {
        _completionNotified = true;
        _buzzer.beepLong();
        Serial.println("COMPLETE");
        return;
    }

    if (next == SystemState::Running) {
        Serial.println("RUN");
        return;
    }

    if (next == SystemState::Idle) {
        Serial.println("IDLE");
    }
}
