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

    //convert the argument to an integer file descriptor
    int fdIn = atoi(argv[1]);
    char buffer[100];

    // --- 1. SET UP TERMINAL ---
    // We MUST do this to read one char at a time
    struct termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio); // Get current settings
    new_tio = old_tio;                 // Copy them

    // Disable canonical mode (waiting for Enter) and echo
    new_tio.c_lflag &= ~(ICANON); 
    new_tio.c_cc[VMIN] = 1;  // Wait for 1 char
    new_tio.c_cc[VTIME] = 0; // Wait forever

    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio); // Apply new settings
    // ----------------------------

    printf("BEGIN GAME!:D\n"); 

    char c;
    char newline = '\n'; // This is your "Enter"

    // --- 2. YOUR LOOP ---
    while (1) 
    {
        // This read() will return *immediately* after one keypress
        if (read(STDIN_FILENO, &c, 1) > 0) 
        {
            // 1. Write the character you typed
            write(fdIn, &c, 1);
            
            // 2. Write the "automatic Enter"
            write(fdIn, &newline, 1);
        
        } 
        else 
        {
            // read() returned 0 (EOF) or -1 (error)
            break;
        }
    }
    // --------------------

    // --- 3. RESTORE TERMINAL ---
    // This is critical, or your terminal will be "broken" after
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    // ---------------------------

    // close(fdW0); // Close your pipe descriptor
    return 0;
}

