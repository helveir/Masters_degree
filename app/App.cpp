#include "App.h"

void App::begin() {
    Serial.begin(9600);

    _buzzer.begin(BUZZER_PIN);
    _rgb.begin(RGB_RED_PIN, RGB_GREEN_PIN, RGB_BLUE_PIN);
    _buttons.begin(BTN_DECREASE_PIN, BTN_START_PIN, BTN_INCREASE_PIN);
    _relay.begin(RELAY_FEED_PIN, RELAY_SUCTION_PIN);
    _motor.begin(MOTOR_IN1_PIN, MOTOR_IN2_PIN);
    _pressureSensor.begin(PRESSURE_SENSOR_PIN, PRESSURE_SENSOR_SIMULATION);

    configureDefaultMode();

    if (!_display.begin()) {
        Serial.println("Display not found");
    }

    _context.pressureRangeKpa = _context.mode.defaultRangeKpa;
    _context.pressure = _pressureSensor.read();

    if (_context.pressure.valid) {
        _context.setState(SystemState::Idle, millis());
        _rgb.setReady();
    } else {
        _context.trip(FaultCode::SensorFault, millis());
        _rgb.setAlarm();
        _buzzer.alarmStart();
    }

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
    _context.mode.minRangeKpa = PRESSURE_RANGE_MIN_KPA;
    _context.mode.maxRangeKpa = PRESSURE_RANGE_MAX_KPA;
    _context.mode.defaultRangeKpa = DEFAULT_PRESSURE_RANGE_KPA;
    _context.mode.toleranceKpa = PRESSURE_TOLERANCE_KPA;
    _context.mode.alarmMarginKpa = PRESSURE_ALARM_MARGIN_KPA;
    _context.mode.maxPhaseTimeMs = MAX_PHASE_TIME_MS;
}

void App::handleButtons(unsigned long nowMs) {
    const bool allowRangeChange =
        _context.state == SystemState::Idle || _context.state == SystemState::Completed;

    if (allowRangeChange) {
        if (_buttons.isIncreasePressed() || _buttons.isIncreaseHeld()) {
            if (_context.pressureRangeKpa < _context.mode.maxRangeKpa) {
                _context.pressureRangeKpa++;
                _buzzer.beepShort();
            }
        }

        if (_buttons.isDecreasePressed() || _buttons.isDecreaseHeld()) {
            if (_context.pressureRangeKpa > _context.mode.minRangeKpa) {
                _context.pressureRangeKpa--;
                _buzzer.beepShort();
            }
        }
    }

    if (_buttons.isStartPressed()) {
        const SystemState previous = _context.state;

        if (_context.state == SystemState::Idle || _context.state == SystemState::Completed) {
            _context.startPressurizing(nowMs);
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
    safetyInput.phaseStartedAtMs = _context.stateStartedAtMs;
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
    controlInput.targetPressureKpa = currentPhaseTargetKpa();
    controlInput.toleranceKpa = _context.mode.toleranceKpa;
    controlInput.depressurizing =
        _context.state == SystemState::Depressurizing ||
        _context.state == SystemState::ReturningToZero;
    const PressureControlResult result = PressureController::compute(controlInput);

    if (_context.state == SystemState::Pressurizing && result.targetReached) {
        const SystemState previous = _context.state;
        _context.startDepressurizing(nowMs);
        _buzzer.beepShort();
        onStateChanged(previous, _context.state);
        ActuatorState switchedActuators;
        applyActuators(switchedActuators);
        return;
    }

    if (_context.state == SystemState::Depressurizing && result.targetReached) {
        const SystemState previous = _context.state;
        _context.startReturningToZero(nowMs);
        _buzzer.beepShort();
        onStateChanged(previous, _context.state);
        ActuatorState switchedActuators;
        applyActuators(switchedActuators);
        return;
    }

    if (_context.state == SystemState::ReturningToZero && result.targetReached) {
        const SystemState previous = _context.state;
        _context.complete(nowMs);
        onStateChanged(previous, _context.state);
        ActuatorState completedActuators;
        applyActuators(completedActuators);
        return;
    }

    applyActuators(result.actuators);
}

int App::currentPhaseTargetKpa() const {
    if (_context.state == SystemState::Depressurizing) {
        return -_context.pressureRangeKpa;
    }

    if (_context.state == SystemState::ReturningToZero) {
        return 0;
    }

    return _context.pressureRangeKpa;
}

void App::applyActuators(const ActuatorState& actuators) {
    _context.actuators = actuators;

    if (actuators.feedOn) {
        _relay.feedOn();
        _relay.suctionOff();
    } else if (actuators.suctionOn) {
        _relay.feedOff();
        _relay.suctionOn();
    } else {
        _relay.bothOff();
    }

    if (actuators.motorOn) {
        _motor.forwardOn();
    } else {
        _motor.stop();
    }
}

void App::render() {
    switch (_context.state) {
        case SystemState::Idle:
        case SystemState::Completed:
            _rgb.setReady();
            break;
        case SystemState::Pressurizing:
        case SystemState::Depressurizing:
        case SystemState::ReturningToZero:
            _rgb.setWorking();
            break;
        case SystemState::Alarm:
            _rgb.setAlarm();
            break;
    }

    _display.update(_context.pressure.pressureKpa,
                    _context.pressureRangeKpa,
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

    if (next == SystemState::Depressurizing) {
        Serial.println("SUCTION");
        return;
    }

    if (next == SystemState::ReturningToZero) {
        Serial.println("ZERO");
        return;
    }

    if (next == SystemState::Completed && !_completionNotified) {
        _completionNotified = true;
        _buzzer.beepLong();
        Serial.println("COMPLETE");
        return;
    }

    if (next == SystemState::Pressurizing) {
        Serial.println("PRESSURE");
        return;
    }

    if (next == SystemState::Idle) {
        Serial.println("IDLE");
    }
}
