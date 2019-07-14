#ifndef SCREEN_H
#define SCREEN_H

#include <SPI.h>
#include "src\epd2in9.h"
#include "src\epdpaint.h"

#define COLORED     0
#define UNCOLORED   1

// Import constants
extern const int screen_update_interval_s;
extern const int max_partial_updates;
extern const bool use_serial_for_debugging;


class Screen {
public:
    Screen();
    ~Screen();
    bool init_screen();
    void update_screen(char* depth);
  
private:
  void DrawString(int x, int y, const char* text, sFONT* font, int colored);
  unsigned char image[864]; // Just enough for one 112pt letter (72*96=6912 pixels or bits => 6912/8 = 864 bytes)
  Paint paint;
  Epd epd;
  char screen_content[5];
  char prev_screen_content[5];
  unsigned long last_screen_update;
  int n_partial_screen_updates_since_full_update = 0;
};

#endif