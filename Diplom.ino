/*
 * ============================================================================
 * ДИПЛОМНЫЙ ПРОЕКТ - УПРАВЛЕНИЕ ДАВЛЕНИЕМ С ОБРАТНОЙ СВЯЗЬЮ
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
// ПОДКЛЮЧЕНИЕ БИБЛИОТЕК И МОДУЛЕЙ
// ============================================================================
#include <Wire.h>              // Библиотека для I2C (нужна до Adafruit_SSD1306)
#include <Adafruit_GFX.h>      // Графическая библиотека
#include <Adafruit_SSD1306.h>  // Библиотека для OLED дисплея

#include "config.h"
#include "Buzzer.h"
#include "RGB_LED.h"
#include "Buttons.h"
#include "Relay.h"

// ============================================================================
// КОНСТАНТЫ ДЛЯ ДИСПЛЕЯ
// ============================================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define OLED_RESET -1

// ============================================================================
// ГЛОБАЛЬНЫЕ ОБЪЕКТЫ
// ============================================================================
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Buzzer buzzer;
RGB_LED rgb;
Buttons buttons;
Relay relay;

// ============================================================================
// ПЕРЕМЕННЫЕ ДЛЯ ТЕСТИРОВАНИЯ
// ============================================================================
int testStage = 0;              // 0-5 этапы тестирования
bool stageActive = false;
unsigned long stageStartTime = 0;

// Данные для отображения на дисплее
int currentPressure = 0;
int targetPressure = 0;
int systemState = 0;            // 0=IDLE, 1=WORKING, 2=ALARM
bool feedActive = false;
bool suctionActive = false;

// Переменные для теста кнопок
bool instructionShown = false;

// Переменные для теста реле
int relaySubStep = 0;
unsigned long relaySubStepStart = 0;

// Переменные для совместного теста
int jointSubStep = 0;
unsigned long jointSubStepStart = 0;
bool headerShown = false;

// ============================================================================
// ПРОТОТИПЫ ФУНКЦИЙ (ОБЪЯВЛЕНИЯ ПЕРЕД ИСПОЛЬЗОВАНИЕМ)
// ============================================================================
void testBuzzer();
void testRGB();
void testButtons();
void testRelay();
void testJoint();
void idleDemo();
void updateDisplay();

// ============================================================================
// SETUP - ВЫПОЛНЯЕТСЯ ОДИН РАЗ ПРИ СТАРТЕ
// ============================================================================
void setup() {
    Serial.begin(9600);
    
    Serial.println(F("========================================="));
    Serial.println(F("   ТЕСТИРОВАНИЕ С OLED ДИСПЛЕЕМ"));
    Serial.println(F("=========================================\n"));
    
    // --- ИНИЦИАЛИЗАЦИЯ ДИСПЛЕЯ ---------------------------------------------
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println(F("OLED дисплей: НЕ НАЙДЕН!"));
        Serial.println(F("  Проверьте подключение: A4(SDA), A5(SCL)"));
    } else {
        Serial.println(F("OLED дисплей: OK"));
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(2);
        display.setCursor(20, 20);
        display.println(F("Diplom"));
        display.setCursor(10, 40);
        display.setTextSize(1);
        display.println(F("Pressure System"));
        display.display();
        delay(2000);
    }
    
    // --- ИНИЦИАЛИЗАЦИЯ ОСТАЛЬНЫХ МОДУЛЕЙ -----------------------------------
    buzzer.begin(BUZZER_PIN);
    rgb.begin(RGB_RED_PIN, RGB_GREEN_PIN, RGB_BLUE_PIN);
    buttons.begin(BTN_DECREASE_PIN, BTN_START_PIN, BTN_INCREASE_PIN);
    relay.begin(RELAY_FEED_PIN, RELAY_SUCTION_PIN);
    
    Serial.println(F("Инициализация модулей завершена\n"));
    
    // --- НАЧАЛЬНЫЕ ЗНАЧЕНИЯ ДЛЯ ДИСПЛЕЯ -----------------------------------
    currentPressure = 0;
    targetPressure = 0;
    systemState = 0;
    feedActive = false;
    suctionActive = false;
    
    updateDisplay();
    
    // Запускаем первый тест
    stageActive = true;
    stageStartTime = millis();
}

// ============================================================================
// LOOP - ГЛАВНЫЙ ЦИКЛ
// ============================================================================
void loop() {
    // --- ОБНОВЛЕНИЕ СОСТОЯНИЙ ----------------------------------------------
    buttons.update();
    buzzer.update();
    
    // --- ОБРАБОТКА ТЕКУЩЕГО ЭТАПА ТЕСТИРОВАНИЯ -----------------------------
    switch(testStage) {
        case 0:
            testBuzzer();
            break;
        case 1:
            testRGB();
            break;
        case 2:
            testButtons();
            break;
        case 3:
            testRelay();
            break;
        case 4:
            testJoint();
            break;
        default:
            idleDemo();
            break;
    }
    
    delay(10);
}

// ============================================================================
// ТЕСТ 0: ЗУММЕР
// ============================================================================
void testBuzzer() {
    if (!stageActive) return;
    
    updateDisplay();
    
    Serial.println(F("[ТЕСТ 1] Зуммер"));
    Serial.println(F("  Короткий писк..."));
    buzzer.beepShort();
    
    if (millis() - stageStartTime >= 500) {
        Serial.println(F("  OK - Зуммер работает\n"));
        testStage = 1;
        stageActive = true;
        stageStartTime = millis();
    }
}

// ============================================================================
// ТЕСТ 1: RGB СВЕТОДИОД
// ============================================================================
void testRGB() {
    if (!stageActive) return;
    
    unsigned long elapsed = millis() - stageStartTime;
    
    if (elapsed < 1000) {
        if (elapsed < 10) Serial.println(F("[ТЕСТ 2] RGB светодиод"));
        rgb.setAlarm();
        systemState = 2;
        updateDisplay();
    }
    else if (elapsed < 2000) {
        rgb.setReady();
        systemState = 0;
        updateDisplay();
        if (elapsed >= 1000 && elapsed < 1010) Serial.println(F("  1. Красный -> АВАРИЯ"));
        if (elapsed >= 1500 && elapsed < 1510) Serial.println(F("  2. Зелёный -> ГОТОВ"));
    }
    else if (elapsed < 3000) {
        rgb.setWorking();
        systemState = 1;
        updateDisplay();
        if (elapsed >= 2000 && elapsed < 2010) Serial.println(F("  3. Синий -> РАБОТА"));
    }
    else if (elapsed < 3500) {
        rgb.setOff();
        if (elapsed >= 3000 && elapsed < 3010) Serial.println(F("  4. Выключение"));
    }
    else {
        Serial.println(F("  OK - RGB работает\n"));
        testStage = 2;
        stageActive = true;
        stageStartTime = millis();
    }
}

// ============================================================================
// ТЕСТ 2: КНОПКИ
// ============================================================================
void testButtons() {
    if (!stageActive) return;
    
    if (!instructionShown) {
        Serial.println(F("[ТЕСТ 3] Кнопки"));
        Serial.println(F("  Нажимайте кнопки для проверки:"));
        Serial.println(F("    - Кнопка START (пин 9) -> синий свет"));
        Serial.println(F("    - Кнопка INCREASE (пин 10) -> короткий писк"));
        Serial.println(F("    - Кнопка DECREASE (пин 6) -> длинный писк"));
        Serial.println(F("  Для завершения теста нажмите и УДЕРЖИВАЙТЕ START 2 секунды\n"));
        instructionShown = true;
    }
    
    if (buttons.isStartPressed()) {
        Serial.println(F("    >> НАЖАТА: START"));
        rgb.setWorking();
        systemState = 1;
        updateDisplay();
        buzzer.beepShort();
    }
    
    if (buttons.isIncreasePressed()) {
        Serial.println(F("    >> НАЖАТА: INCREASE (+)"));
        buzzer.beepShort();
        targetPressure++;
        if (targetPressure > 5) targetPressure = 5;
        updateDisplay();
    }
    
    if (buttons.isDecreasePressed()) {
        Serial.println(F("    >> НАЖАТА: DECREASE (-)"));
        buzzer.beepLong();
        targetPressure--;
        if (targetPressure < -5) targetPressure = -5;
        updateDisplay();
    }
    
    if (buttons.isIncreaseHeld()) {
        if (millis() - stageStartTime >= 2000) {
            Serial.println(F("  Завершение теста кнопок..."));
            Serial.println(F("  OK - Кнопки работают\n"));
            testStage = 3;
            stageActive = true;
            stageStartTime = millis();
            instructionShown = false;
            rgb.setReady();
            systemState = 0;
            updateDisplay();
        }
    } else {
        stageStartTime = millis();
    }
}

// ============================================================================
// ТЕСТ 3: РЕЛЕ
// ============================================================================
void testRelay() {
    if (!stageActive) return;
    
    if (relaySubStep == 0) {
        Serial.println(F("[ТЕСТ 4] Реле (клапаны)"));
        Serial.println(F("  ВНИМАНИЕ! Реле будут включаться/выключаться!"));
        relaySubStep = 1;
        relaySubStepStart = millis();
    }
    
    unsigned long elapsed = millis() - relaySubStepStart;
    
    switch(relaySubStep) {
        case 1:
            if (elapsed < 10) {
                relay.feedOn();
                feedActive = true;
                suctionActive = false;
                Serial.println(F("  1. ПОДАЧА ВКЛ (2 секунды)"));
                rgb.setAlarm();
                systemState = 2;
                updateDisplay();
            }
            if (elapsed >= 2000) {
                relay.feedOff();
                feedActive = false;
                updateDisplay();
                relaySubStep = 2;
                relaySubStepStart = millis();
            }
            break;
            
        case 2:
            if (elapsed < 10) {
                Serial.println(F("  2. ПОДАЧА ВЫКЛ (1 секунда)"));
                rgb.setReady();
                systemState = 0;
                updateDisplay();
            }
            if (elapsed >= 1000) {
                relaySubStep = 3;
                relaySubStepStart = millis();
            }
            break;
            
        case 3:
            if (elapsed < 10) {
                relay.suctionOn();
                feedActive = false;
                suctionActive = true;
                Serial.println(F("  3. ОТСОС ВКЛ (2 секунды)"));
                rgb.setAlarm();
                systemState = 2;
                updateDisplay();
            }
            if (elapsed >= 2000) {
                relay.suctionOff();
                suctionActive = false;
                updateDisplay();
                relaySubStep = 4;
                relaySubStepStart = millis();
            }
            break;
            
        case 4:
            if (elapsed < 10) {
                Serial.println(F("  4. ОТСОС ВЫКЛ (1 секунда)"));
                rgb.setReady();
                systemState = 0;
                updateDisplay();
            }
            if (elapsed >= 1000) {
                relaySubStep = 5;
                relaySubStepStart = millis();
            }
            break;
            
        case 5:
            if (elapsed < 10) {
                relay.feedOn();
                relay.suctionOn();
                feedActive = true;
                suctionActive = true;
                Serial.println(F("  5. ОБА КЛАПАНА ВКЛ (2 секунды)"));
                rgb.setWorking();
                systemState = 1;
                updateDisplay();
            }
            if (elapsed >= 2000) {
                relay.bothOff();
                feedActive = false;
                suctionActive = false;
                updateDisplay();
                relaySubStep = 6;
                relaySubStepStart = millis();
            }
            break;
            
        case 6:
            if (elapsed < 10) {
                Serial.println(F("  6. ОБА КЛАПАНА ВЫКЛ"));
                Serial.println(F("  OK - Реле работают\n"));
            }
            if (elapsed >= 500) {
                testStage = 4;
                stageActive = true;
                stageStartTime = millis();
                relaySubStep = 0;
            }
            break;
    }
}

// ============================================================================
// ТЕСТ 4: СОВМЕСТНАЯ РАБОТА
// ============================================================================
void testJoint() {
    if (!stageActive) return;
    
    if (!headerShown) {
        Serial.println(F("[ТЕСТ 5] СОВМЕСТНАЯ РАБОТА ВСЕХ МОДУЛЕЙ"));
        headerShown = true;
        jointSubStep = 0;
        jointSubStepStart = millis();
    }
    
    unsigned long elapsed = millis() - jointSubStepStart;
    
    switch(jointSubStep) {
        case 0:
            if (elapsed < 10) {
                buzzer.beepShort();
                rgb.setReady();
                systemState = 0;
                feedActive = false;
                suctionActive = false;
                currentPressure = 0;
                updateDisplay();
                Serial.println(F("  Этап 0: Старт"));
            }
            if (elapsed >= 1000) {
                jointSubStep = 1;
                jointSubStepStart = millis();
            }
            break;
            
        case 1:
            if (elapsed < 10) {
                relay.feedOn();
                feedActive = true;
                buzzer.beepLong();
                rgb.setWorking();
                systemState = 1;
                currentPressure = 3;
                updateDisplay();
                Serial.println(F("  Этап 1: ПОДАЧА"));
            }
            if (elapsed >= 2500) {
                jointSubStep = 2;
                jointSubStepStart = millis();
            }
            break;
            
        case 2:
            if (elapsed < 10) {
                relay.feedOff();
                feedActive = false;
                relay.suctionOn();
                suctionActive = true;
                rgb.setAlarm();
                systemState = 2;
                buzzer.alarmStart();
                currentPressure = -3;
                updateDisplay();
                Serial.println(F("  Этап 2: ОТСОС + АВАРИЯ"));
            }
            if (elapsed >= 3000) {
                relay.suctionOff();
                suctionActive = false;
                buzzer.alarmStop();
                jointSubStep = 3;
                jointSubStepStart = millis();
            }
            break;
            
        case 3:
            if (elapsed < 10) {
                buzzer.beepShort();
                rgb.setReady();
                systemState = 0;
                currentPressure = 0;
                updateDisplay();
                Serial.println(F("  Этап 3: Завершение"));
            }
            if (elapsed >= 1500) {
                Serial.println(F("\n========================================="));
                Serial.println(F("     ВСЕ ТЕСТЫ УСПЕШНО ЗАВЕРШЕНЫ!"));
                Serial.println(F("=========================================\n"));
                testStage = 5;
                stageActive = false;
                headerShown = false;
                jointSubStep = 0;
                rgb.setReady();
                systemState = 0;
                updateDisplay();
            }
            break;
    }
}

// ============================================================================
// IDLE DEMO - ДЕМОНСТРАЦИЯ РАБОТЫ ДИСПЛЕЯ ПОСЛЕ ТЕСТОВ
// ============================================================================
void idleDemo() {
    static unsigned long lastUpdate = 0;
    static int pressureDir = 1;
    
    // Имитация изменения давления (для демонстрации дисплея)
    if (millis() - lastUpdate >= 100) {
        lastUpdate = millis();
        
        currentPressure += pressureDir;
        if (currentPressure >= 5) {
            currentPressure = 5;
            pressureDir = -1;
        }
        if (currentPressure <= -5) {
            currentPressure = -5;
            pressureDir = 1;
        }
        
        updateDisplay();
    }
    
    // Обработка нажатий для изменения уставки
    if (buttons.isIncreasePressed()) {
        targetPressure++;
        if (targetPressure > 5) targetPressure = 5;
        updateDisplay();
        buzzer.beepShort();
        Serial.print(F("Уставка: "));
        Serial.println(targetPressure);
    }
    
    if (buttons.isDecreasePressed()) {
        targetPressure--;
        if (targetPressure < -5) targetPressure = -5;
        updateDisplay();
        buzzer.beepShort();
        Serial.print(F("Уставка: "));
        Serial.println(targetPressure);
    }
    
    if (buttons.isStartPressed()) {
        Serial.println(F("Старт/Стоп (будет реализовано позже)"));
        buzzer.beepShort();
    }
}

// ============================================================================
// UPDATE DISPLAY - ОБНОВЛЕНИЕ ЭКРАНА
// ============================================================================
void updateDisplay() {
    display.clearDisplay();
    
    // Рамка
    display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    
    // Строка состояния
    display.setTextSize(1);
    display.setCursor(2, 2);
    switch(systemState) {
        case 0: display.print(F("IDLE")); break;
        case 1: display.print(F("WORKING")); break;
        case 2: display.print(F("ALARM")); break;
    }
    
    // Давление и уставка
    display.setCursor(65, 2);
    display.print(F("P:"));
    display.print(currentPressure);
    display.print(F("kPa"));
    
    display.setCursor(65, 12);
    display.print(F("S:"));
    display.print(targetPressure);
    display.print(F("kPa"));
    
    // Шкала давления
    int barX = 10;
    int barY = 35;
    int barWidth = 108;
    int barHeight = 10;
    int minPressure = -5;
    int maxPressure = 5;
    int range = maxPressure - minPressure;
    
    display.drawRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);
    
    // Текущее давление
    int pressurePos = ((currentPressure - minPressure) * barWidth) / range;
    if (pressurePos < 0) pressurePos = 0;
    if (pressurePos > barWidth) pressurePos = barWidth;
    display.fillRect(barX, barY, pressurePos, barHeight, SSD1306_WHITE);
    
    // Маркер уставки
    int setpointPos = ((targetPressure - minPressure) * barWidth) / range;
    int markerX = barX + setpointPos;
    display.fillTriangle(markerX - 2, barY - 2, markerX + 2, barY - 2, markerX, barY - 6, SSD1306_WHITE);
    
    // Подписи к шкале
    display.setCursor(barX - 4, barY + 3);
    display.print(minPressure);
    display.setCursor(barX + barWidth - 8, barY + 3);
    display.print(maxPressure);
    
    // Клапаны
    display.setCursor(2, SCREEN_HEIGHT - 12);
    display.print(feedActive ? F("FEED:ON ") : F("FEED:OFF"));
    display.print(suctionActive ? F("SUC:ON") : F("SUC:OFF"));
    
    display.display();
}