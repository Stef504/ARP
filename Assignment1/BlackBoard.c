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
#include <stdbool.h>

#define MAX_ITEMS 20
typedef struct {
    int x;
    int y;
} Point;

// Global variables and parameters
int window_width = 0;
int window_height = 0;
int rph_intial = 0;
double eta_intial = 0.0;
int force_intial = 0;
int mass = 1;        
int k_intial = 0;
int working_area = 0;
int t_intial = 100;  
int H = 0, W = 0;
int wh = 0, ww = 0;
bool running = true;

Point obstacles[MAX_ITEMS];
int obs_head = 0;
int obs_count = 0;

Point targets[MAX_ITEMS];
int tar_head = 0;
int tar_count = 0;

// Function to read parameter file
void Parameter_File() {
    FILE* file = fopen("Parameter_File.txt", "r");
    if (file == NULL) {
        perror("Error opening Parameter_File.txt");
        return;
    }

    char line[256];
    int line_number = 0;

    // Reading file line by line
    while (fgets(line, sizeof(line), file)) {
        line_number++;

        // Converting lines from parameter file into arrays of words
        // The words are separated by a token defined in the parameter file.
        char* tokens[10]; 
        int token_count = 0;
        char* token = strtok(line, "_");

        while (token != NULL && token_count < 10) {
            tokens[token_count] = token;
            token_count++;
            token = strtok(NULL, "_"); 
        }

        // Assign the respective values to the global parameters
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
    wh = (H > 6) ? H - 6 : H;
    ww = (W > 10) ? W - 10 : W;
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

    int term_h, term_w;
    getmaxyx(stdscr, term_h, term_w);
    /* use parameter file values but clamp to terminal size */
    int win_h = (window_height > 0) ? window_height : term_h;
    int win_w = (window_width  > 0) ? window_width  : term_w;
    if (win_h > term_h) win_h = term_h;
    if (win_w > term_w) win_w = term_w;

    /* newwin(nlines, ncols, y, x) -> newwin(height, width, ...) */
    WINDOW *win = newwin(win_w, win_h, 0, 0);
    layout_and_draw(win);

    if (argc < 4) 
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
    char sIn[10], sOb[10], sTa[10];
    char format_stringIn[100] = "%s";
    char format_stringOb[100] = "%d,%d";
    char format_stringTa[100] = "%d,%d";
    int x_coord_Ob, y_coord_Ob;
    int x_coord_Ta, y_coord_Ta;
    timeout(t_intial);

    fd_set readfds;
    int maxfd = fdIn;
    if (fdOb > maxfd) maxfd = fdOb;
    if (fdTa > maxfd) maxfd = fdTa;

    // Persistent Coordinates (Initialize off-screen or valid default)
    // Removed single coordinates in favor of arrays
                   
    int newH = H - wh;
    int newW = W - ww;

    float x_curr = newW / 2.0;
    float x_prev = newW / 2.0;
    float x_prev2 = newW / 2.0;

    float y_curr = newH / 2.0;
    float y_prev = newH / 2.0;
    float y_prev2 = newH / 2.0;

    if(running == false){
        exit(0);
    }

    while (running) {
        int ch = getch(); // Non-blocking due to timeout()

        if (ch == KEY_RESIZE) {
            // Update ncurses internal structures for new dimensions
            resize_term(0, 0);
            layout_and_draw(win);
        }

        // Clear window for new frame
        werase(win);
        box(win, 0, 0);

        FD_ZERO(&readfds);
        FD_SET(fdIn, &readfds);
        FD_SET(fdOb, &readfds);
        FD_SET(fdTa, &readfds);

        // FIX: Reset timer every loop
        tv.tv_sec = 0;
        tv.tv_usec = 0; // Use a zero-timeout for select to poll

        retval = select(maxfd + 1, &readfds, NULL, NULL, &tv);

        if (retval == -1) {
            break;
        } 
        else if (retval > 0) {
            // Receiving data from input pipe
            if (FD_ISSET(fdIn, &readfds)) {
                ssize_t bytes = read(fdIn, strIn, sizeof(strIn)-1);
                if (bytes > 0) {
                    strIn[bytes] = '\0';
                    sscanf(strIn, "%s", sIn);
                } else { running = false; } 
            }

            // Receiving coordinates from obstacle pipe
            if (FD_ISSET(fdOb, &readfds)) {
                ssize_t bytes = read(fdOb, strOb, sizeof(strOb)-1);
                if (bytes > 0) {
                    strOb[bytes] = '\0';
                    int new_x, new_y;
                    sscanf(strOb, format_stringOb, &new_x, &new_y);
                    // Clamp to window dimensions to prevent vanishing
                    if (new_x >= ww - 1) new_x = ww - 2;
                    if (new_y >= wh - 1) new_y = wh - 2;
                    
                    // Store in array
                    obstacles[obs_head].x = new_x;
                    obstacles[obs_head].y = new_y;
                    obs_head = (obs_head + 1) % MAX_ITEMS;
                    if (obs_count < MAX_ITEMS) obs_count++;
                }
            }

            // Reading coordinates from target pipe
            if (FD_ISSET(fdTa, &readfds)) {
                ssize_t bytes = read(fdTa, strTa, sizeof(strTa)-1);
                if (bytes > 0) {
                    strTa[bytes] = '\0';
                    int new_x, new_y;
                    sscanf(strTa, format_stringTa, &new_x, &new_y);
                    // Clamp to window dimensions to prevent vanishing
                    if (new_x >= ww - 1) new_x = ww - 2;
                    if (new_y >= wh - 1) new_y = wh - 2;

                    // Store in array
                    targets[tar_head].x = new_x;
                    targets[tar_head].y = new_y;
                    tar_head = (tar_head + 1) % MAX_ITEMS;
                    if (tar_count < MAX_ITEMS) tar_count++;
                }
            }
        }                

        float Fx = 0;
        float Fy = 0;
        float Fxy = 0;
        float diag_force = (float)force_intial * M_SQRT1_2;

        if (sIn[0]=='q'){
            running = false;
        }
        if (sIn[0]== 'a'){
            mvwprintw(win, term_h/2,term_w/2, "+");
            mvwprintw(win, term_h/2,term_w/2, " ");
            x_curr=term_w/2;
            y_curr=term_h/2;
            x_prev=term_w/2;
            y_prev=term_h/2;
            x_prev2=term_w/2;
            y_prev2=term_h/2;
            wrefresh(win);
        }

        if (sIn[0] == 'p') {
            mvwprintw(win, 0, 0, "Game Paused, Press 'u' (via pipe) to Resume");
            mvwprintw(win, (int)y_curr, (int)x_curr, "+");
            wrefresh(win);

            fd_set pause_fds;
            struct timeval pause_tv;
            int pause_ret;
            int pause_ch = -1;

            while (running) {
                pause_ch = getch();
                if (pause_ch == 'u') break;

                FD_ZERO(&pause_fds);
                FD_SET(fdIn, &pause_fds);
                FD_SET(fdOb, &pause_fds);
                FD_SET(fdTa, &pause_fds);
                pause_tv.tv_sec = 0;
                pause_tv.tv_usec = 100 * 1000; // 100 ms

                pause_ret = select(maxfd + 1, &pause_fds, NULL, NULL, &pause_tv);
                if (pause_ret > 0) {
                    // If input pipe has data, read and update sIn
                    if (FD_ISSET(fdIn, &pause_fds)) {
                        ssize_t bytes = read(fdIn, strIn, sizeof(strIn) - 1);
                        if (bytes > 0) {
                            strIn[bytes] = '\0';
                            sscanf(strIn, "%s", sIn);
                            if (sIn[0] == 'u') break;
                        } else {
                            running = false;
                            break;
                        }
                    }
                    // Still consume obstacle/target updates so they remain visible
                    if (FD_ISSET(fdOb, &pause_fds)) {
                        ssize_t bytes = read(fdOb, strOb, sizeof(strOb) - 1);
                        if (bytes > 0) {
                            strOb[bytes] = '\0';
                            int new_x, new_y;
                            sscanf(strOb, "%d,%d", &new_x, &new_y);
                            if (new_x >= ww - 1) new_x = ww - 2;
                            if (new_y >= wh - 1) new_y = wh - 2;
                            obstacles[obs_head].x = new_x;
                            obstacles[obs_head].y = new_y;
                            obs_head = (obs_head + 1) % MAX_ITEMS;
                            if (obs_count < MAX_ITEMS) obs_count++;
                        }
                    }
                    if (FD_ISSET(fdTa, &pause_fds)) {
                        ssize_t bytes = read(fdTa, strTa, sizeof(strTa) - 1);
                        if (bytes > 0) {
                            strTa[bytes] = '\0';
                            int new_x, new_y;
                            sscanf(strTa, "%d,%d", &new_x, &new_y);
                            if (new_x >= ww - 1) new_x = ww - 2;
                            if (new_y >= wh - 1) new_y = wh - 2;
                            targets[tar_head].x = new_x;
                            targets[tar_head].y = new_y;
                            tar_head = (tar_head + 1) % MAX_ITEMS;
                            if (tar_count < MAX_ITEMS) tar_count++;
                        }
                    }
                }

                // redraw the pause message/frame so UI stays alive
                werase(win);
                box(win, 0, 0);
                mvwprintw(win, 0, 0, "Game Paused, Press 'u' to Resume");
                for(int i=0; i<obs_count; i++) {
                     if (obstacles[i].x > 0 && obstacles[i].y > 0) 
                        mvwprintw(win, obstacles[i].y, obstacles[i].x, "O");
                }
                for(int i=0; i<tar_count; i++) {
                     if (targets[i].x > 0 && targets[i].y > 0) 
                        mvwprintw(win, targets[i].y, targets[i].x, "T");
                }
                mvwprintw(win, (int)y_curr, (int)x_curr, "+");
                wrefresh(win);
            }

            // Clear pause message after resume
            mvwprintw(win, 0, 0, "                             ");
            mvwprintw(win, (int)y_curr, (int)x_curr, "+");
            wrefresh(win);
        }
        

        // 3. INPUT MAPPING (The Fix)
        // We apply +/- signs directly to Fx and Fy here.
        switch (sIn[0]) {
            // STRAIGHT
            case 'e': Fy = -force_intial; break; // Up
            case 'c': Fy =  force_intial; break; // Down
            case 's': Fx = -force_intial; break; // Left
            case 'f': Fx =  force_intial; break; // Right
            
            // DIAGONAL (Split the force into X and Y components)
            case 'w': // Up-Left (-X, -Y)
                Fx = -diag_force; 
                Fy = -diag_force; 
                break;
            case 'r': // Up-Right (+X, -Y)
                Fx =  diag_force; 
                Fy = -diag_force; 
                break;
            case 'x': // Down-Left (-X, +Y)
                Fx = -diag_force; 
                Fy =  diag_force; 
                break;
            case 'v': // Down-Right (+X, +Y)
                Fx =  diag_force; 
                Fy =  diag_force; 
                break;
            
            case 'q': running = false; break;
        }

        // 2. PHYSICS CALCULATION (The Equation)
        // We calculate X and Y independently to allow 8-direction movement.
        
        float T= t_intial / 1000.0; // Convert ms to seconds
        
        // Denominator is the same for both: (M + KT)
        float denom = mass + (k_intial * T);
        
        // Constant term for history: (2M + KT)
        float history_factor = (2 * mass) + (k_intial * T);

        // Formula: x_i = [ F*T^2 + x_{i-1}*(2M+KT) - M*x_{i-2} ] / (M+KT)
        float num_x = (Fx * T * T) + (x_prev * history_factor) - (mass * x_prev2);
        x_curr = num_x / denom;

        float num_y = (Fy * T * T) + (y_prev * history_factor) - (mass * y_prev2);
        y_curr = num_y / denom;


        // 3. WALL COLLISION (No Wrap-Around)
        // If we hit a wall, we clamp the position and reset history 
        // to kill the momentum (otherwise it sticks/vibrates).
        
        if (x_curr >= ww - 1) {
            x_curr = ww - 1;
            x_prev = x_curr; x_prev2 = x_curr; // Stop momentum
        } else if (x_curr <= 0) {
            x_curr = 0;
            x_prev = x_curr; x_prev2 = x_curr; // Stop momentum
        }

        if (y_curr >= wh - 1) {
            y_curr = wh - 1;
            y_prev = y_curr; y_prev2 = y_curr; // Stop momentum
        } else if (y_curr <= 0) {
            y_curr = 0;
            y_prev = y_curr; y_prev2 = y_curr; // Stop momentum
        }

        // --- 4. DRAWING ---
        // Draw Obstacles
        for(int i=0; i<obs_count; i++) {
             if (obstacles[i].x > 0 && obstacles[i].y > 0) 
                mvwprintw(win, obstacles[i].y, obstacles[i].x, "O");
        }

        // Draw Targets
        for(int i=0; i<tar_count; i++) {
             if (targets[i].x > 0 && targets[i].y > 0) 
                mvwprintw(win, targets[i].y, targets[i].x, "T");
        }

        // Draw the drone
        mvwprintw(win, (int)y_curr, (int)x_curr, "+");

        // --- 5. SHIFT HISTORY (Prepare for next loop) ---
        x_prev2 = x_prev;
        y_prev2 = y_prev;
        x_prev = x_curr;
        y_prev = y_curr;
        
        // --- 6. REFRESH SCREEN ---
        wrefresh(win);

        // --- 7. FRAME DELAY ---
        usleep(t_intial * 1000);
    }
    
    // Cleanup
    delwin(win);
    endwin();
    return 0;

}





