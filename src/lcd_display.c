#include "lcd_display.h"
#include <stdio.h>
#include <stdlib.h>

void display_init() {
    // Initialize display (clear screen for terminal version)
    system("clear"); // For Windows. Use system("clear") for Unix/Linux.
}

void display_message(const char* message) {
    printf("%s\n", message);
}

void display_character(char c) {
    switch(c) {
        case 'U': 
            printf("UP "); 
            break;
        case 'D': 
            printf("DOWN "); 
            break;
        case 'L': 
            printf("LEFT "); 
            break;
        case 'R': 
            printf("RIGHT "); 
            break;
        default: 
            return;
    }
    fflush(stdout); // Force output to be displayed immediately
}
