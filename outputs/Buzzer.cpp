#include "Buzzer.h"

void Buzzer::begin(uint8_t pin) {
    _pin = pin;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    
    _beepActive = false;
    _alarmActive = false;
    _alarmState = false;
    _lastAlarmToggle = 0;
    
    // Для пассивного зуммера: частота 2000 Гц (2 кГц)
    // Это приятный, хорошо слышимый тон
    _alarmOnTime = 250;
    _alarmOffTime = 250;
}

// КОРОТКИЙ ПИСК — ИСПРАВЛЕНО
void Buzzer::beepShort() {
    _beepActive = true;
    _beepStartTime = millis();
    _beepDuration = 150;  // 150 мс
    
    // ВКЛЮЧАЕМ ЗВУК через tone() — генерирует частоту
    // 2000 Гц — частота звука, 150 — длительность (неблокирующая!)
    tone(_pin, 2000, _beepDuration);
    
    // tone() НЕ блокирует выполнение, звук сам выключится через _beepDuration мс
    // Поэтому флаг _beepActive можно сбросить по таймеру
}

// ДЛИННЫЙ ПИСК — ИСПРАВЛЕНО
void Buzzer::beepLong() {
    _beepActive = true;
    _beepStartTime = millis();
    _beepDuration = 500;  // 500 мс
    
    tone(_pin, 1500, _beepDuration);  // Более низкий тон (1500 Гц)
}

void Buzzer::alarmStart() {
    _alarmActive = true;
    _lastAlarmToggle = millis();
    _alarmState = true;
    // НЕ включаем звук сразу — update() будет управлять
}

void Buzzer::alarmStop() {
    _alarmActive = false;
    noTone(_pin);  // ОТКЛЮЧАЕМ tone()
    digitalWrite(_pin, LOW);
}

void Buzzer::update() {
    unsigned long now = millis();
    
    // Для короткого/длинного писка: просто ждём окончания
    if (_beepActive) {
        if (now - _beepStartTime >= _beepDuration) {
            _beepActive = false;
            // tone уже сам выключился, но на всякий случай:
            noTone(_pin);
        }
        return;  // Пока писк — авария не обрабатывается
    }
    
    // Аварийный сигнал (прерывистый)
    if (_alarmActive) {
        unsigned long interval = _alarmState ? _alarmOnTime : _alarmOffTime;
        
        if (now - _lastAlarmToggle >= interval) {
            _lastAlarmToggle = now;
            _alarmState = !_alarmState;
            
            if (_alarmState) {
                tone(_pin, 2500);  // Включаем высокий тон для аварии
            } else {
                noTone(_pin);      // Выключаем
            }
        }
    }
}

bool Buzzer::isAlarmActive() {
    return _alarmActive;
}
