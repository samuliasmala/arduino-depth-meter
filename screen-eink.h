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
extern const bool display_debugging_on_screen;


class Screen {
public:
    Screen();
    ~Screen();
    bool init_screen();
    void update_screen(char* depth);
  
private:
  void DrawString(int x, int y, const char* text, sFONT* font, int colored);
  // Just enough for one 112pt letter (72*(96+8)=7488 pixels or bits => 7488/8 = 936 bytes)
  // +8 in above comes from epd.SetFrameMemory where x coordinate must be multiple of 8 which
  // means movements under that must be done in paint canvas
  unsigned char image[936];
  Paint paint;
  Epd epd;
  char screen_content[5];
  char prev_screen_content[5];
  char screen_debug[50];
  unsigned long last_screen_update;
  int n_partial_screen_updates_since_full_update = 0;
  int total_screen_updates = 0;
};

#endif
