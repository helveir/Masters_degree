#include "Buttons.h"

// ============================================================================
// BEGIN - ИНИЦИАЛИЗАЦИЯ ВСЕХ КНОПОК
// ============================================================================
void Buttons::begin(uint8_t decreasePin, uint8_t startPin, uint8_t increasePin) {
    // --- СОХРАНЯЕМ НОМЕРА ПИНОВ В СООТВЕТСТВУЮЩИЕ СТРУКТУРЫ ---------------
    // Оператор . (точка) используется для доступа к полям структуры
    _btnDecrease.pin = decreasePin;   // Пин 6 для кнопки уменьшения
    _btnStart.pin = startPin;         // Пин 9 для кнопки старт/стоп
    _btnIncrease.pin = increasePin;   // Пин 10 для кнопки увеличения
    
    // --- НАСТРОЙКА ПИНОВ КАК ВХОДОВ С ВНУТРЕННЕЙ ПОДТЯЖКОЙ ---------------
    // INPUT_PULLUP - режим, при котором пин внутренним резистором подтянут к +5V
    // Когда кнопка НЕ НАЖАТА - на пине HIGH (5V)
    // Когда кнопка НАЖАТА (замыкает на GND) - на пине LOW (0V)
    // Это экономит внешние резисторы и даёт чёткое состояние
    pinMode(_btnDecrease.pin, INPUT_PULLUP);
    pinMode(_btnStart.pin, INPUT_PULLUP);
    pinMode(_btnIncrease.pin, INPUT_PULLUP);
    
    // --- ИНИЦИАЛИЗАЦИЯ КНОПКИ УМЕНЬШЕНИЯ -----------------------------------
    _btnDecrease.lastState = HIGH;        // Последнее прочитанное значение - HIGH (не нажата)
    _btnDecrease.currentState = HIGH;     // Текущее стабильное состояние - HIGH
    _btnDecrease.wasPressed = false;      // Нажатие ещё не обработано
    _btnDecrease.lastDebounceTime = 0;    // Время последнего изменения
    // --- ИНИЦИАЛИЗАЦИЯ КНОПКИ СТАРТА --------------------------------------
    _btnStart.lastState = HIGH;
    _btnStart.currentState = HIGH;
    _btnStart.wasPressed = false;
    _btnStart.lastDebounceTime = 0;
    // --- ИНИЦИАЛИЗАЦИЯ КНОПКИ УВЕЛИЧЕНИЯ ----------------------------------
    _btnIncrease.lastState = HIGH;
    _btnIncrease.currentState = HIGH;
    _btnIncrease.wasPressed = false;
    _btnIncrease.lastDebounceTime = 0;
    
    // --- ОБНУЛЯЕМ ТАЙМЕРЫ УДЕРЖАНИЯ ---------------------------------------
    // 0 означает "кнопка не удерживается"
    _lastIncreaseHold = 0;
    _lastDecreaseHold = 0;
    _lastIncreaseRepeat = 0;
    _lastDecreaseRepeat = 0;
}

// ============================================================================
// UPDATE BUTTON - ОБРАБОТКА АНТИДРЕБЕЗГА ДЛЯ ОДНОЙ КНОПКИ
// ============================================================================
// Принимает ссылку на структуру Button (Button& btn)
// & означает, что мы работаем с оригиналом, а не с копией
void Buttons::updateButton(Button& btn) {
    // --- ЧИТАЕМ ТЕКУЩЕЕ СОСТОЯНИЕ ПИНА ------------------------------------
    // digitalRead() возвращает HIGH (1) или LOW (0)
    bool reading = digitalRead(btn.pin);
    
    // --- ПРОВЕРЯЕМ: ИЗМЕНИЛОСЬ ЛИ СОСТОЯНИЕ? ------------------------------
    // Если currentState отличается от того, что только что прочитали
    if (reading != btn.lastState) {
        // Да, изменилось! Запоминаем время этого изменения
        // Это может быть дребезг (ложное срабатывание)
        btn.lastDebounceTime = millis();
    }
    
    // --- ПРОВЕРЯЕМ: ПРОШЛО ЛИ ДОСТАТОЧНО ВРЕМЕНИ? -------------------------
    // Если с момента последнего изменения прошло больше DEBOUNCE_DELAY (50 мс)
    if ((millis() - btn.lastDebounceTime) > DEBOUNCE_DELAY) {
        // Да, сигнал стабилизировался. Можно доверять этому значению.
        if (reading != btn.currentState) {
            // Состояние действительно изменилось (не дребезг)
            btn.currentState = reading;
            
            // Если кнопка была нажата (LOW), сбрасываем флаг wasPressed
            // Это позволяет зарегистрировать новое нажатие при следующем is...Pressed()
            if (btn.currentState == LOW) {
                btn.wasPressed = false;   // Новое нажатие — ещё не обработано
            }
        }
    }
    
    // --- СОХРАНЯЕМ ПОСЛЕДНЕЕ ПРОЧИТАННОЕ СОСТОЯНИЕ ------------------------
    // Нужно для следующей итерации, чтобы определить изменение
    btn.lastState = reading;
}

// ============================================================================
// UPDATE - ОБНОВЛЕНИЕ ВСЕХ КНОПОК
// ============================================================================
void Buttons::update() {
    // Вызываем updateButton() для каждой кнопки
    // Каждый вызов обрабатывает антидребезг для одной кнопки
    updateButton(_btnDecrease);   // Обрабатываем кнопку уменьшения
    updateButton(_btnStart);      // Обрабатываем кнопку старта
    updateButton(_btnIncrease);   // Обрабатываем кнопку увеличения
}

// ============================================================================
// IS START PRESSED - ПРОВЕРКА НАЖАТИЯ КНОПКИ СТАРТА (ОДНОКРАТНО)
// ============================================================================
bool Buttons::isStartPressed() {
    // Проверяем:
    // 1. currentState == LOW - кнопка физически нажата (замкнута на GND)
    // 2. wasPressed == false - мы ещё НЕ сообщали программе об этом нажатии
    if (_btnStart.currentState == LOW && !_btnStart.wasPressed) {
        // Ставим флаг, что нажатие обработано
        _btnStart.wasPressed = true;
        // Возвращаем true - да, было нажатие
        return true;
    }
    // Если условий нет - возвращаем false
    return false;
}

// ============================================================================
// IS INCREASE PRESSED - ПРОВЕРКА НАЖАТИЯ КНОПКИ УВЕЛИЧЕНИЯ
// ============================================================================
bool Buttons::isIncreasePressed() {
    if (_btnIncrease.currentState == LOW && !_btnIncrease.wasPressed) {
        _btnIncrease.wasPressed = true;
        return true;
    }
    return false;
}

// ============================================================================
// IS DECREASE PRESSED - ПРОВЕРКА НАЖАТИЯ КНОПКИ УМЕНЬШЕНИЯ
// ============================================================================
bool Buttons::isDecreasePressed() {
    if (_btnDecrease.currentState == LOW && !_btnDecrease.wasPressed) {
        _btnDecrease.wasPressed = true;
        return true;
    }
    return false;
}

// ============================================================================
// IS INCREASE HELD - ПРОВЕРКА УДЕРЖАНИЯ КНОПКИ УВЕЛИЧЕНИЯ (АВТОПОВТОР)
// ============================================================================
// Этот метод позволяет: нажал и держишь - значение меняется с ускорением
// Сначала ничего не происходит HOLD_DELAY мс, затем каждые HOLD_INTERVAL мс
bool Buttons::isIncreaseHeld() {
    if (_btnIncrease.currentState == LOW) {
        if (_lastIncreaseHold == 0) {
            _lastIncreaseHold = millis();
            _lastIncreaseRepeat = 0;
            return false;   // Первое нажатие не считаем автоповтором
        }

        unsigned long now = millis();
        if (now - _lastIncreaseHold < HOLD_DELAY) {
            return false;
        }

        if (_lastIncreaseRepeat == 0 || now - _lastIncreaseRepeat >= HOLD_INTERVAL) {
            _lastIncreaseRepeat = now;
            return true;
        }
    } else {
        _lastIncreaseHold = 0;
        _lastIncreaseRepeat = 0;
    }
    return false;
}

// ============================================================================
// IS DECREASE HELD - ПРОВЕРКА УДЕРЖАНИЯ КНОПКИ УМЕНЬШЕНИЯ
// ============================================================================
// Абсолютно такая же логика, как для кнопки увеличения
bool Buttons::isDecreaseHeld() {
    if (_btnDecrease.currentState == LOW) {
        if (_lastDecreaseHold == 0) {
            _lastDecreaseHold = millis();
            _lastDecreaseRepeat = 0;
            return false;
        }

        unsigned long now = millis();
        if (now - _lastDecreaseHold < HOLD_DELAY) {
            return false;
        }

        if (_lastDecreaseRepeat == 0 || now - _lastDecreaseRepeat >= HOLD_INTERVAL) {
            _lastDecreaseRepeat = now;
            return true;
        }
    } else {
        _lastDecreaseHold = 0;
        _lastDecreaseRepeat = 0;
    }
    return false;
}
