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
#include <math.h>

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
bool running = true;

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
    
    if (x_coord != 0 && y_coord != 0){
        mvwprintw(win, y_coord, x_coord, "%s", "O");
        wrefresh(win);
        sleep(1);
    }
}

void target_generation(WINDOW* win, int x_coord, int y_coord){
    
    if (x_coord != 0 && y_coord != 0){
        mvwprintw(win, y_coord, x_coord, "%s", "T");
        wrefresh(win);
        sleep(2);
    }
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
    timeout(t_intial);

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
                sscanf(strIn, format_stringIn, sIn);
                // TODO: Process keyboard imput
               // wrefresh(win);
            }
            
        }

        int ch=getch();

        while ((sIn[0]) != 'q') {
            if (ch == KEY_RESIZE) {
                // Update ncurses internal structures for new dimensions
                resize_term(0, 0);          // o resizeterm(0, 0)
                layout_and_draw(win);       // calcullate layout and draw again
            }
        // ... Obastcles and Targets ...
        srand(time(NULL));
        x_coord_Ob = rand() % (W - 10);
        y_coord_Ob = rand() % (H - 10);

        srand(time(NULL) + 1);
        x_coord_Ta = rand() % (W - 10);
        y_coord_Ta = rand() % (H - 10);

        obstacle_generation(win, y_coord_Ob, x_coord_Ob);
        target_generation(win, y_coord_Ta, x_coord_Ta);

        //.....Drone.....
        // --- INITIALIZATION ---
        // We use floats for physics precision, cast to int for drawing.
        // Start in the middle of the screen.
        mvwinstr(win, H/2, W/2, 'H');
        float x_curr = maxX / 2.0;
        float x_prev = maxX / 2.0;
        float x_prev2 = maxX / 2.0;

        float y_curr = maxY / 2.0;
        float y_prev = maxY / 2.0;
        float y_prev2 = maxY / 2.0;
        


        if (sIn[0]== 'a'){

            mvwprintw(win,H/2, W/2, 'H');
            wrefresh(win);
        }
        if (sIn[0] == 'p'){
            while (sIn[0] != 'u'){   
                mvprintw(0, 0, "Game Paused, Press 'u' to Resume");
                wrefresh(win);
                obstacle_generation(win, 0, 0);
                target_generation(win, 0, 0);
                sleep(2);
            }

        }
        if (sIn[0]=='u'){
            mvwaddch(win, 0, 0, ' ');
            wrefresh(win);

        }
        
        
        // Reset force to 0 at start of every frame.
        // If key is held, force is applied. If released, force becomes 0.
        float Fx = 0;
        float Fy = 0;

        switch (sIn[0]) {
            case 'e': Fy = -Force; break; // Up (Negative Y)
            case 'c': Fy = Force;  break; // Down (Positive Y)
            case 's': Fx = -Force; break; // Left (Negative X)
            case 'f': Fx = Force;  break; // Right (Positive X)
            case 'w': Fxy = Force; break; // Up-Right
            case 'x': Fxy = -Force; break; // Down-Left
            case 'r': Fxy = Force; break; // Up-Left
            case 'v': Fxy = -Force; break; // Down-Right
            case KEY_ESC: running = false; break;
        }

        // 2. PHYSICS CALCULATION (The Equation)
        // We calculate X and Y independently to allow 8-direction movement.
        
        // Denominator is the same for both: (M + KT)
        float denom = M + (K * T);
        
        // Constant term for history: (2M + KT)
        float history_factor = (2 * M) + (K * T);

        // Formula: x_i = [ F*T^2 + x_{i-1}*(2M+KT) - M*x_{i-2} ] / (M+KT)
        float num_x = (Fx * T * T) + (x_prev * history_factor) - (M * x_prev2);
        x_curr = num_x / denom;

        float num_y = (Fy * T * T) + (y_prev * history_factor) - (M * y_prev2);
        y_curr = num_y / denom;

        //diagonal
        while (sIn[0] == 'w' || sIn[0] == 'x' || sIn[0] == 'r' || sIn[0] == 'v') {
        float num_x = (Fxy * T * T) + (x_prev * history_factor) - (M * x_prev2);
        x_curr = num_x / denom;

        float num_y = (Fxy * T * T) + (y_prev * history_factor) - (M * y_prev2);
        y_curr = num_y / denom;

        // 4. DRAWING
        clear();
        mvprintw((int)(y_curr+x_curr), (int)(x_curr+y_curr), "H");
        }

        // 3. WALL COLLISION (No Wrap-Around)
        // If we hit a wall, we clamp the position and reset history 
        // to kill the momentum (otherwise it sticks/vibrates).
        
        if (x_curr >= maxX - 1) {
            x_curr = maxX - 1;
            x_prev = x_curr; x_prev2 = x_curr; // Stop momentum
        } else if (x_curr <= 0) {
            x_curr = 0;
            x_prev = x_curr; x_prev2 = x_curr; // Stop momentum
        }

        if (y_curr >= maxY - 1) {
            y_curr = maxY - 1;
            y_prev = y_curr; y_prev2 = y_curr; // Stop momentum
        } else if (y_curr <= 0) {
            y_curr = 0;
            y_prev = y_curr; y_prev2 = y_curr; // Stop momentum
        }

        // 4. DRAWING
        clear();
        mvprintw((int)y_curr, (int)x_curr, "H");

        // 5. SHIFT HISTORY (Prepare for next loop)
        // Oldest becomes older
        x_prev2 = x_prev;
        y_prev2 = y_prev;
        
        // Current becomes previous
        x_prev = x_curr;
        y_prev = y_curr;
    
    }

    }

    // Cleanup
    delwin(win);
    endwin();
    return 0;
}





