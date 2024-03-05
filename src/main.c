#include "game_logic.h"
#include "lcd_display.h"
#include "button_input.h"


int main() {
    init_game();
    while (!is_game_over()) {
        update_game();
    }
    return 0;
}
