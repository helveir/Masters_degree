#include "Display.h"

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ ДИСПЛЕЯ
// ============================================================================
bool Display::begin() {
    // Пытаемся подключиться к дисплею
    if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        _connected = false;
        return false;   // Дисплей не найден
    }
    
    _connected = true;
    
    // Настройки экрана
    _display.clearDisplay();                    // Очищаем буфер
    _display.setTextColor(SSD1306_WHITE);       // Белый текст на чёрном фоне
    
    return true;
}

// ============================================================================
// ОБНОВЛЕНИЕ ЭКРАНА - ТОЛЬКО КРУПНЫЕ ЦИФРЫ
// ============================================================================
void Display::update(int pressure, int setpoint, int state, 
                     bool feed, bool suction) {
    if (!_connected) return;
    
    _display.clearDisplay();
    
    // ===== СТРОКА 1: ДАВЛЕНИЕ (САМОЕ КРУПНОЕ) =====
    _display.setTextSize(3);                    // Огромный шрифт (18x24 пикселя)
    _display.setCursor(10, 5);                  // Позиция X=10, Y=5
    _display.print(pressure);                   // Выводим цифру давления
    
    _display.setTextSize(2);                    // Маленький шрифт для "kPa"
    _display.print(F(" kPa"));                  // Единицы измерения
    
    // ===== СТРОКА 2: СОСТОЯНИЕ СИСТЕМЫ =====
    _display.setTextSize(2);                    // Средний шрифт (12x16)
    _display.setCursor(10, 35);                 // Ниже давления
    
    switch(state) {
        case 0: _display.print(F("IDLE")); break;    // Ожидание
        case 1: _display.print(F("PUMP")); break;    // Накачка
        case 2: _display.print(F("ALARM")); break;   // Авария
        case 3: _display.print(F("SUCT")); break;    // Отсос
        case 4: _display.print(F("ZERO")); break;    // Возврат к атмосфере
        case 5: _display.print(F("DONE")); break;    // Завершено
        default: _display.print(F("ERR")); break;
    }
    
    // ===== СТРОКА 3: УСТАВКА ВНИЗУ =====
    _display.setTextSize(2);                    // Маленький шрифт для уставки
    _display.setCursor(10, 50);                 // В самом низу
    _display.print(F("+/-"));                  // Симметричная граница
    _display.print(setpoint);                   // Значение уставки
    _display.print(F(" kPa"));
    
    // ===== ОТПРАВЛЯЕМ НА ЭКРАН =====
    _display.display();
}
