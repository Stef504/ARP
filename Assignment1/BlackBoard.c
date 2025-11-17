#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#define _XOPEN_SOURCE_EXTENDED
#include <locale.h>
#include <ncurses.h>

int rph_intial;
double eta_intial;
int force_intial;
int mass;
int k_intial;
int working_area;
int t_intial;

void Parameter_File() {
    FILE* file = fopen("Parameter_File.txt", "r");
    if (file == NULL) {
        perror("Error opening Parameter_File.txt");
        return;
    }

    char line[256];
    int line_number = 0;

    // --- 2. Read the file line by line ---
    while (fgets(line, sizeof(line), file)) {
        line_number++;

        // --- 3. Tokenize the *current* line ---
        char* tokens[10]; // An array to hold tokens for this ONE line
        int token_count = 0;
        char* token = strtok(line, "_");

        while (token != NULL && token_count < 10) {
            tokens[token_count] = token; // Add token to our array
            token_count++;
            token = strtok(NULL, "_"); // Get next token
        }

        // --- 4. Assign values based on line number ---
        // We use a 'switch' to make it cleaner than many 'if' statements.
        // We also check 'token_count' to avoid crashing if a line is blank.
        switch (line_number) {
            case 3:
                if (token_count > 2) rph_intial = atoi(tokens[2]);
                break;
            case 4:
                if (token_count > 2) eta_intial = atof(tokens[2]); // Use atof() for doubles
                break;
            case 5:
                if (token_count > 2) force_intial = atoi(tokens[2]);
                break;
            case 6:
                if (token_count > 1) mass = atoi(tokens[1]); // You used index [1] here
                break;
            case 7:
                if (token_count > 2) k_intial = atoi(tokens[2]);
                break;
            case 8:
                if (token_count > 2) working_area = atoi(tokens[2]);
                break;
            case 9:
                if (token_count > 2) t_intial = atoi(tokens[2]);
                break;
        }
    }
    fclose(file);
}

static void layout_and_draw(WINDOW *win) {
    int H, W;
    getmaxyx(stdscr, H, W);

    // Window with fixed margin
    int wh = (H > 6) ? H - 6 : H;
    int ww = (W > 10) ? W - 10 : W;
    if (wh < 3) wh = 3;
    if (ww < 3) ww = 3;

    // Resize and recenter window
    wresize(win, wh, ww);
    mvwin(win, (H - wh) / 2, (W - ww) / 2);

    // Clean up and draw again
    werase(stdscr);
    werase(win);
    box(win, 0, 0);
   

    refresh();
    wrefresh(win);
}
  
int main(int argc, char *argv[]) {

    Parameter_File();
    
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Create initial window (Dimensions TBM)
    WINDOW *win = newwin(3, 3, 0, 0);
    layout_and_draw(win);

    if (argc < 3) 
    {
        fprintf(stderr, "Usage: %s <fd>\n", argv[0]);
        endwin();
        exit(1);
    }

    // Convert the argument to an integer file descriptor
    int fdIn = atoi(argv[1]);
    char bufferIn[100];

    int fdOb = atoi(argv[2]);
    char bufferOb[100];

    int fdTa = atoi(argv[3]);
    char bufferTa[100];

    struct timeval tv;
    int retval;
    char strIn[100], strOb[100], strTa[100];
    char sIn[10], sOb[100], sTa[100];
    char format_stringIn[100] = "%s";
    char format_stringOb[100] = "%d,%d";
    char format_stringTa[100] = "%d,%d";
    int x_coord_Ob, y_coord_Ob;
    int x_coord_Ta, y_coord_Ta;

    fd_set readfds;

    int maxfd = fdIn;
    if (fdOb > maxfd) maxfd = fdOb;
    if (fdTa > maxfd) maxfd = fdTa;

    while (1){
        FD_ZERO(&readfds);
        FD_SET(fdIn, &readfds);
        FD_SET(fdOb, &readfds);
        FD_SET(fdTa, &readfds);

        tv.tv_sec = 5;
        tv.tv_usec = 0;
        retval = select(maxfd + 1, &readfds, NULL, NULL, &tv);
 
        if (retval == -1) {
            perror("select()");
        } else if (retval > 0) {
            // Data from Ob pipeline
            if (FD_ISSET(fdOb, &readfds)) 
            {
                ssize_t bytesOb = read(fdOb, strOb, sizeof(strOb)-1); 
                
                if (bytesOb <= 0) 
                {
                    printf("Pipe closed\n");
                    break;
                }

                strOb[bytesOb] = '\0';
                sscanf(strOb, format_stringOb, &x_coord_Ob, &y_coord_Ob);
                mvwprintw(win, x_coord_Ob, y_coord_Ob, "%s", "O");
                wrefresh(win);
            }

            // Data from Ta pipeline
            if (FD_ISSET(fdTa, &readfds)) 
            {
                ssize_t bytesTa = read(fdTa, strTa, sizeof(strTa)-1); 
                
                if (bytesTa <= 0) 
                {
                    printf("Pipe closed\n");
                    break;
                }

                strTa[bytesTa] = '\0';
                sscanf(strTa, format_stringTa, &x_coord_Ta, &y_coord_Ta);
                mvwprintw(win, x_coord_Ta, y_coord_Ta, "%s", "T");
                wrefresh(win);
            }
        }
    }

    // Cleanup
    delwin(win);
    endwin();
    return 0;
}





