#include "screen-lcd.h"

Screen_lcd::Screen_lcd() :
  lcd(10, 3, 7, 6, 5, 4)
{
}

Screen_lcd::~Screen_lcd() {
}

bool Screen_lcd::init_screen() {
  // set up the number of columns and rows on the LCD
  lcd.begin(16, 2);
  return true;
}


void Screen_lcd::update_screen(char* depth) {
  snprintf(screen_content, 20, "Depth: %s m", depth);

  // Update screen only if there has been a change
  if(strcmp(screen_content, prev_screen_content) != 0 || millis() - last_screen_update > screen_update_interval_s*1000) {
    // Update lcd screen with the value stored in depth variable
    // prev_screen_content is used to update screen only if there is a change (prevents blinking)
    // clean up the screen before printing a new reply
    // lcd.clear();
    // set the cursor to column 0, line 0
    lcd.setCursor(0, 0);
    // print line 1
    lcd.print(screen_content);
 
    last_screen_update = millis();
  }
  strncpy(prev_screen_content, screen_content, 20);
}
