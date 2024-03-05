// game_logic.h
#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <signal.h>

void init_game();
void update_game();
int is_game_over();
void add_to_sequence();
void display_sequence();
void level_up();
void handle_timeout(int sig);
void display_level_infos();

#endif
