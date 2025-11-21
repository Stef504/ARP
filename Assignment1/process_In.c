#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <sys/wait.h>
#include <curses.h>
#include <sys/time.h>
#include <termios.h>

int main(int argc, char *argv[]) 
{

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fd>\n", argv[0]);
        exit(1);
    }

    //Convert the argument to an integer file descriptor
    int fdIn = atoi(argv[1]);
    char buffer[100];

    // Setting up the terminal to read single characters without waiting for Enter
    struct termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio); 
    new_tio = old_tio;                 

    // Disable canonical mode (waiting for Enter) and echo
    new_tio.c_lflag &= ~(ICANON); 
    new_tio.c_cc[VMIN] = 1;  
    new_tio.c_cc[VTIME] = 0; 
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio); 

    printf("BEGIN GAME!:D\n"); 

    // Characters to be written
    // One is the typed character, the other is the "automatic Enter"
    char c;
    char newline = '\n'; 

    while (1) 
    {
        if (read(STDIN_FILENO, &c, 1) > 0) 
        {
            write(fdIn, &c, 1);
            write(fdIn, &newline, 1);
            if (c == 'q') {
                break;
            }
        } 
        else 
        {
            break;
        }

    }

    // Restore the old terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    return 0;
}

