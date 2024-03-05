#include "game_logic.h"
#include "button_input.h"
#include "lcd_display.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SEQUENCE_LENGTH 4
#define MAX_INT 65536
#define LEVEL_TIME 6 * MAX_SEQUENCE_LENGTH

static int current_length = 1;
static int input_index = 0;
static int game_over = 0;
static int level = 1;
static char sequence[65535]; // max 16 bits
static int score = 0;
static int new_level_flag = 1;

void init_game() {
    srand(time(NULL));
    display_init();
    input_init();
    display_level_infos();
    add_to_sequence();
    display_sequence();
    signal(SIGALRM, handle_timeout); // signal pour le timeout
}

void update_game() {
    if (input_index < current_length) {
        char input = get_input();
        if (input != 0) {
            if (new_level_flag == 1){
                alarm(level + LEVEL_TIME);
            }
            new_level_flag = 0;
            if (input == sequence[input_index]) {
                input_index++;
                if (input_index == current_length) {
                    display_message("Correct!");
                    score += 1;
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
    if (current_length == (MAX_SEQUENCE_LENGTH + level * 2)){
        display_message("Level Up!");
        level_up();
        display_sequence();
        new_level_flag = 1;
    }
}

void level_up() {
    level++;
    current_length = 1; // Reset sequence length for new level
    display_level_infos();
    alarm(level + LEVEL_TIME); // adjust new level time
}

void handle_timeout(int sig) {
    if (sig == SIGALRM) {
        display_message("Time's Up! Game Over!");
        game_over = 1;
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

void display_level_infos(){
    clear_console();
    printf("****************************\n");
    printf("Niveau actuel : %d\n", level);
    display_score(score);
    display_timer(LEVEL_TIME + level);
    printf("****************************\n");
}
