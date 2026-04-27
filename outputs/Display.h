#ifndef DISPLAY_H
#define DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================================================================
// НАСТРОЙКИ ДИСПЛЕЯ
// ============================================================================
#define SCREEN_WIDTH 128    // Ширина экрана в пикселях
#define SCREEN_HEIGHT 64    // Высота экрана в пикселях
#define OLED_ADDR 0x3C      // I2C адрес дисплея (0x3C или 0x3D)
#define OLED_RESET -1       // Пин сброса (-1 = не используется)

// ============================================================================
// КЛАСС DISPLAY - УПРАВЛЕНИЕ ЭКРАНОМ
// ============================================================================
class Display {
  public:
    bool begin();                                          // Инициализация
    void update(int pressure, int setpoint, int state, 
                bool feed, bool suction);                  // Обновление экрана
    
  private:
    Adafruit_SSD1306 _display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    bool _connected = false;
};

#endif
