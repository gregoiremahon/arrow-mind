#include "button_input.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

void input_init() {
    // A AJOUTER POUR VERSION CARTE
}

int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    // Get current terminal attributes
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    
    // Set terminal to non-blocking and non-canonical mode
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);

    // Set stdin to non-blocking mode
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    // Restore terminal attributes and mode
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

char get_input() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    
    if (kbhit()) {
        buf = getchar();
    }
    
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror ("tcsetattr ~ICANON");
    
    switch (buf) {
        case 'A': return 'U'; // Arrow up
        case 'B': return 'D'; // Arrow down
        case 'D': return 'L'; // Arrow left
        case 'C': return 'R'; // Arrow right
        default: return 0; // No input or unrecognized input
    }
}
