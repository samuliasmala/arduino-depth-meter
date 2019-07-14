#ifndef SCREEN_LCD_H
#define SCREEN_LCD_H

// Load lcd library and initialize with the numbers of the interface pins
#include <Arduino.h>
#include <LiquidCrystal.h>

// Import constants
extern const int screen_update_interval_s;
extern const bool use_serial_for_debugging;


class Screen_lcd {
public:
    Screen_lcd();
    ~Screen_lcd();
    bool init_screen();
    void update_screen(char* depth);
  
private:
  LiquidCrystal lcd;
  char screen_content[20];
  char prev_screen_content[20];
  unsigned long last_screen_update;
};

#endif