#include "screen-eink.h"

Screen::Screen() :
  paint(image, 0, 0) // width should be the multiple of 8 
{
}


Screen::~Screen() {
}


bool Screen::init_screen() {
  if (epd.Init(lut_full_update) != 0) {
      if(use_serial_for_debugging)
        Serial.print("e-Paper init failed");
      return false;
  }

  /** 
   *  there are 2 memory areas embedded in the e-paper display
   *  and once the display is refreshed, the memory area will be auto-toggled,
   *  i.e. the next action of SetFrameMemory will set the other memory area
   *  therefore you have to clear the frame memory twice.
   */
  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();
  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black

  if (epd.Init(lut_partial_update) != 0) {
      if(use_serial_for_debugging)
         Serial.print("e-Paper init failed");
      return false;
  }
  
  return true;
}


void Screen::update_screen(char* depth) {
  strncpy(screen_content, depth, 5);

  // Update screen only if there has been a change or the screen update interval has passed
  if(strcmp(screen_content, prev_screen_content) != 0 || millis() - last_screen_update > screen_update_interval_s*1000) {
    total_screen_updates++;
    n_partial_screen_updates_since_full_update++;
    
    // Check if full screen update required
    bool full_update = false;
    if(n_partial_screen_updates_since_full_update >= max_partial_updates) {
      epd.SetLut(lut_full_update);
      epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
      full_update = true;
    }
    
    if(display_debugging_on_screen) {      
      snprintf(screen_debug, 50, "Screen updates: %-5d", total_screen_updates);
      this->DrawString(116, 0, screen_debug, &Font12, COLORED);
      snprintf(screen_debug, 50, "Until refresh: %-5d", max_partial_updates-n_partial_screen_updates_since_full_update);
      this->DrawString(116, 160, screen_debug, &Font12, COLORED);
    }
    
    this->DrawString(16, 4, screen_content, &CourierNew112Bold, COLORED);
    epd.DisplayFrame();

    if(full_update) {
      n_partial_screen_updates_since_full_update = 0;
      epd.SetLut(lut_partial_update);
      if(use_serial_for_debugging)
        Serial.println("Full screen update done");
    }


    last_screen_update = millis();
  }
  strncpy(prev_screen_content, screen_content, 5);
}


void Screen::DrawString(int x, int y, const char* text, sFONT* font, int colored) {
    const char* p_text = text;
    unsigned int counter = 0;
    int refcolumn = x;

    // Initialize paint area
    paint.SetWidth(font->Height);
    paint.SetHeight(font->Width);
    paint.SetRotate(ROTATE_90);

/* Send the string character by character on EPD */
    while (*p_text != 0) {
      paint.Clear(UNCOLORED);
      paint.DrawCharAt(0, 0, *p_text, font, colored);
      epd.SetFrameMemory(paint.GetImage(), x, y+font->Width*counter, paint.GetWidth(), paint.GetHeight());

      /* Point on the next character */
      p_text++;
      counter++;
    }
}
