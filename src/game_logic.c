#include "game_logic.h"
#include "button_input.h"
#include "lcd_display.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SEQUENCE_LENGTH 20

static char sequence[MAX_SEQUENCE_LENGTH];
static int current_length = 1;
static int input_index = 0;
static int game_over = 0;

void init_game() {
    srand(time(NULL));
    display_init();
    input_init();
    add_to_sequence();
    display_sequence();
}


void update_game() {
    if (input_index < current_length) {
        char input = get_input();
        if (input != 0) {
            if (input == sequence[input_index]) {
                input_index++;
                if (input_index == current_length) {
                    display_message("Correct!");
                    current_length++;
                    input_index = 0;
                    add_to_sequence();
                    display_sequence();
                }
            } else {
                display_message("Game Over!");
                game_over = 1;
            }
        }
    }
}

int is_game_over() {
    return game_over;
}

void add_to_sequence() {
    int index = rand() % 4;
    char directions[4] = {'U', 'D', 'L', 'R'};
    sequence[current_length - 1] = directions[index];
}

void display_sequence() {
    for (int i = 0; i < current_length; i++) {
        display_character(sequence[i]);
    }
}
