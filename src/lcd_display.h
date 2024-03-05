// lcd_display.h
#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

void display_init();
void display_message(const char* message);
void display_character(char c);
void display_timer(int seconds);
void display_score(int score);
void clear_console();

#endif
