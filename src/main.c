#include "game_logic.h"
#include "lcd_display.h"
#include "button_input.h"

int main() {
    init_game();
    while (1) {
        update_game();
    }
    return 0;
}
