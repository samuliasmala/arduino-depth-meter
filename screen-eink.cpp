#include "screen-eink.h"

Screen::Screen() :
  paint(image, 0, 0)
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

    // Draw screen content to middle of screen
    // (widths and heights mixed because of screen rotation)
    int x = (EPD_WIDTH - eink_font.Height)/2;
    int y = (EPD_HEIGHT - strlen(screen_content)*eink_font.Width)/2;

    this->DrawString(x, y, screen_content, &eink_font, COLORED);
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

    // Initialize paint area
    // 1 byte = 8 pixels, so this should be the multiple of 8
    // Sub-8 changes must be included in canvas coordinates so +8 required
    paint.SetWidth(font->Height + 8);
    paint.SetHeight(font->Width);
    paint.SetRotate(ROTATE_90);

/* Send the string character by character on EPD */
    while (*p_text != 0) {
      paint.Clear(UNCOLORED);
      paint.DrawCharAt(0, x%8, *p_text, font, colored);
      /* 1 byte = 8 pixels, so the x should be the multiple of 8. Sub-8 changes must be included in DrawCharAt coordinates*/
      epd.SetFrameMemory(paint.GetImage(), x - x%8, y+font->Width*counter, paint.GetWidth(), paint.GetHeight());

      /* Point on the next character */
      p_text++;
      counter++;
    }
}
