/*
 * ============================================================================
 * ДИПЛОМНЫЙ ПРОЕКТ - СИСТЕМА УПРАВЛЕНИЯ ДАВЛЕНИЕМ
 * ============================================================================
 * 
 * Модули:
 * - Зуммер (пин 3)
 * - RGB светодиод (пины 2,7,8)
 * - Кнопки (пины 6,9,10)
 * - Реле клапанов (пины 11,12)
 * - OLED дисплей 128x64 (I2C: A4=SDA, A5=SCL)
 * 
 * ============================================================================
 */

// ============================================================================
// ПОДКЛЮЧЕНИЕ МОДУЛЕЙ
// ============================================================================
#include "config.h"
#include "Buzzer.h"
#include "RGB_LED.h"
#include "Buttons.h"
#include "Relay.h"
#include "Display.h"

// ============================================================================
// СОЗДАНИЕ ОБЪЕКТОВ
// ============================================================================
Buzzer buzzer;
RGB_LED rgb;
Buttons buttons;
Relay relay;
Display display;

// ============================================================================
// ПЕРЕМЕННЫЕ СИСТЕМЫ
// ============================================================================
int currentPressure = 0;      // Текущее давление (кПа)
int targetPressure = 0;       // Уставка (целевое давление)
int systemState = 0;          // 0=IDLE, 1=WORKING, 2=ALARM
bool feedActive = false;      // Клапан подачи включён?
bool suctionActive = false;   // Клапан отсоса включён?

// Для имитации давления (позже замените на реальный датчик)
unsigned long lastPressureUpdate = 0;

// ============================================================================
// НАСТРОЙКА (ВЫПОЛНЯЕТСЯ ОДИН РАЗ)
// ============================================================================
void setup() {
    Serial.begin(9600);
    
    // Инициализация всех модулей
    buzzer.begin(BUZZER_PIN);
    rgb.begin(RGB_RED_PIN, RGB_GREEN_PIN, RGB_BLUE_PIN);
    buttons.begin(BTN_DECREASE_PIN, BTN_START_PIN, BTN_INCREASE_PIN);
    relay.begin(RELAY_FEED_PIN, RELAY_SUCTION_PIN);
    
    // Инициализация дисплея
    if (!display.begin()) {
        Serial.println("Дисплей не найден!");
    } else {
        Serial.println("Дисплей готов");
    }
    
    // Начальное состояние
    rgb.setReady();
    systemState = 0;
    display.update(currentPressure, targetPressure, systemState, feedActive, suctionActive);
    
    Serial.println("Система готова. Нажмите START");
}

// ============================================================================
// ГЛАВНЫЙ ЦИКЛ (ВЫПОЛНЯЕТСЯ БЕСКОНЕЧНО)
// ============================================================================
void loop() {
    // ===== ОБНОВЛЕНИЕ МОДУЛЕЙ =====
    buttons.update();    // Опрос кнопок (антидребезг)
    buzzer.update();     // Обновление состояния зуммера
    
    // ===== ОБРАБОТКА КНОПОК =====
    
    // Кнопка START/STOP
    if (buttons.isStartPressed()) {
        if (systemState == 0) {
            // ЗАПУСК СИСТЕМЫ
            systemState = 1;
            rgb.setWorking();
            buzzer.beepShort();
            Serial.println("СИСТЕМА ЗАПУЩЕНА");
        } else {
            // ОСТАНОВ СИСТЕМЫ
            systemState = 0;
            relay.bothOff();
            feedActive = false;
            suctionActive = false;
            rgb.setReady();
            buzzer.beepShort();
            Serial.println("СИСТЕМА ОСТАНОВЛЕНА");
        }
        display.update(currentPressure, targetPressure, systemState, feedActive, suctionActive);
    }
    
    // Кнопка УВЕЛИЧЕНИЕ уставки
    if (buttons.isIncreasePressed()) {
        targetPressure++;
        if (targetPressure > 5) targetPressure = 5;
        buzzer.beepShort();
        display.update(currentPressure, targetPressure, systemState, feedActive, suctionActive);
        Serial.print("Уставка: ");
        Serial.println(targetPressure);
    }
    
    // Кнопка УМЕНЬШЕНИЕ уставки
    if (buttons.isDecreasePressed()) {
        targetPressure--;
        if (targetPressure < -5) targetPressure = -5;
        buzzer.beepShort();
        display.update(currentPressure, targetPressure, systemState, feedActive, suctionActive);
        Serial.print("Уставка: ");
        Serial.println(targetPressure);
    }
    
    // Удержание кнопки УВЕЛИЧЕНИЕ (быстрая смена)
    if (buttons.isIncreaseHeld()) {
        targetPressure++;
        if (targetPressure > 5) targetPressure = 5;
        display.update(currentPressure, targetPressure, systemState, feedActive, suctionActive);
        delay(50);
    }
    
    // Удержание кнопки УМЕНЬШЕНИЕ (быстрая смена)
    if (buttons.isDecreaseHeld()) {
        targetPressure--;
        if (targetPressure < -5) targetPressure = -5;
        display.update(currentPressure, targetPressure, systemState, feedActive, suctionActive);
        delay(50);
    }
    
    // ===== ЛОГИКА УПРАВЛЕНИЯ (ПОКА ИМИТАЦИЯ) =====
    if (systemState == 1) {
        // Имитация изменения давления каждые 100 мс
        if (millis() - lastPressureUpdate > 100) {
            lastPressureUpdate = millis();
            
            if (currentPressure < targetPressure) {
                currentPressure++;
                feedActive = true;
                suctionActive = false;
                relay.feedOn();
                relay.suctionOff();
            }
            else if (currentPressure > targetPressure) {
                currentPressure--;
                feedActive = false;
                suctionActive = true;
                relay.feedOff();
                relay.suctionOn();
            }
            else {
                feedActive = false;
                suctionActive = false;
                relay.bothOff();
            }
            
            // Обновляем дисплей при изменении давления
            display.update(currentPressure, targetPressure, systemState, feedActive, suctionActive);
        }
    }
    
    delay(10);  // Небольшая задержка для стабильности
}