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
#include <sys/time.h>
#include <time.h>

int window_width;
int window_height;
int rph_intial;
double eta_intial;
int force_intial;
int mass;
int k_intial;
int working_area;
int t_intial;
int H, W;

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
            case 1:
                if (token_count > 2) window_width = atoi(tokens[2]);
                break;
            case 2:
                if (token_count > 2) window_height = atoi(tokens[2]);
                break;
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

void obstacle_generation(WINDOW* win, int x_coord, int y_coord){
    mvwprintw(win, y_coord, x_coord, "%s", "O");
    wrefresh(win);
    sleep(1);
}

void target_generation(WINDOW* win, int x_coord, int y_coord){
    mvwprintw(win, y_coord, x_coord, "%s", "T");
    wrefresh(win);
    sleep(2);
}
  
int main(int argc, char *argv[]) {

    Parameter_File();
    
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int term_h, term_w;
    getmaxyx(stdscr, term_h, term_w);
    /* use parameter file values but clamp to terminal size */
    int win_h = (window_height > 0) ? window_height : term_h;
    int win_w = (window_width  > 0) ? window_width  : term_w;
    if (win_h > term_h) win_h = term_h;
    if (win_w > term_w) win_w = term_w;

    /* newwin(nlines, ncols, y, x) -> newwin(height, width, ...) */
    WINDOW *win = newwin(win_h, win_w, 0, 0);
    layout_and_draw(win);

    if (argc < 2) 
    {
        fprintf(stderr, "Usage: %s <fd>\n", argv[0]);
        endwin();
        exit(1);
    }

    // Convert the argument to an integer file descriptor
    int fdIn = atoi(argv[1]);
    char bufferIn[100];

    struct timeval tv;
    int retval;
    char strIn[100], sIn[10];
    char format_stringIn[100] = "%s";
    int x_coord_Ob, y_coord_Ob;
    int x_coord_Ta, y_coord_Ta;

    fd_set readfds;
    int maxfd = fdIn;

    while (1){
        FD_ZERO(&readfds);
        FD_SET(fdIn, &readfds);

        tv.tv_sec = 5;
        tv.tv_usec = 0;
        retval = select(maxfd + 1, &readfds, NULL, NULL, &tv);

        if (retval == -1) {
            perror("select()");
        } else if (retval > 0) {
            if(FD_ISSET(fdIn, &readfds)) 
            {
                ssize_t bytesIn = read(fdIn, strIn, sizeof(strIn)-1); 
                
                if (bytesIn <= 0) 
                {
                    printf("Pipe closed\n");
                    break;
                }

                strIn[bytesIn] = '\0';
                // TODO: Process keyboard imput
                wrefresh(win);
            }
            
        }
        int ch;
        while ((ch = getch()) != 'q') {
            if (ch == KEY_RESIZE) {
                // Update ncurses internal structures for new dimensions
                resize_term(0, 0);          // o resizeterm(0, 0)
                layout_and_draw(win);       // calcullate layout and draw again
            }
        // ... other buttons ...
        srand(time(NULL));
        x_coord_Ob = rand() % (W - 10);
        y_coord_Ob = rand() % (H - 10);

        srand(time(NULL) + 1);
        x_coord_Ta = rand() % (W - 10);
        y_coord_Ta = rand() % (H - 10);

        obstacle_generation(win, y_coord_Ob, x_coord_Ob);
        target_generation(win, y_coord_Ta, x_coord_Ta);
    }
    


    }

    // Cleanup
    delwin(win);
    endwin();
    return 0;
}





