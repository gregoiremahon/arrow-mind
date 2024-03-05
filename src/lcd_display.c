#include "lcd_display.h"
#include <stdio.h>
#include <stdlib.h>

void display_init() {
    // Initialize display (clear screen for terminal version)
    system("clear"); // For Windows. Use system("clear") for Unix/Linux.
}

void clear_console(){
    system("clear");
}

void display_message(const char* message) {
    printf("%s\n", message);
}

void display_character(char c) {
    switch(c) {
        case 'U': 
            printf("HAUT "); 
            break;
        case 'D': 
            printf("BAS "); 
            break;
        case 'L': 
            printf("GAUCHE "); 
            break;
        case 'R': 
            printf("DROITE "); 
            break;
        default: 
            return;
    }
    fflush(stdout);
}

void display_timer(int seconds) {
    printf("Temps disponible pour ce niveau: %02d sec\n", seconds);
    fflush(stdout);
}

void display_score(int score){
    printf("Score: %d\n", score);
    fflush(stdout);
}
