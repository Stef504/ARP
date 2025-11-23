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
#include <errno.h>
#define MAX_ITEMS 20
typedef struct {
    int x;
    int y;
} Point;

// Global variables and parameters
int window_width ;
int window_height;
int rph_intial;
double eta_intial;
int force_intial ;
int mass;        
int k_intial ;
int working_area;
int t_intial;  
int H = 0, W = 0;
int wh = 0, ww = 0;
bool running = true;
bool skip_drone_update = false;

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
    // allow window to report keys / KEY_RESIZE without blocking
    keypad(win, TRUE);
    wtimeout(win, 50); // wait up to 50 ms in wgetch, then continue to select()

    if (argc < 6) 
    {
        fprintf(stderr, "Usage: %s <fd>\n", argv[0]);
        endwin();
        exit(1);
    }

    // Convert the argument to an integer file descriptor
    int fdToBB = atoi(argv[1]);   // Drone Dynamics (Read)
    int fdFromBB = atoi(argv[2]);   // Drone Dynamics (Write - for init)
    int fdOb = atoi(argv[3]);    // Obstacle
    int fdTa = atoi(argv[4]);    // Target
    char *path_bb = argv[5];
    int fdIn_BB = open(path_bb, O_RDONLY | O_NONBLOCK);
    if (fdIn_BB == -1) { perror("Failed to open BB Pipe"); return 1; }

    struct timeval tv;
    int retval;
    char strToBB[135],strFromBB[135], strOb[100], strTa[100], strIn[100]; 
    char sToBB[135],sFromBB[135], sOb[135], sTa[135],sIn[10];
    char format_stringIn[100] = "%s";
    char format_stringOb[100] = "%d,%d";
    char format_stringTa[100] = "%d,%d";
    int x_coord_Ob, y_coord_Ob;
    int x_coord_Ta, y_coord_Ta;
    

    fd_set readfds;

    int maxfd = fdToBB;
    if (fdOb > maxfd) maxfd = fdOb;
    if (fdTa > maxfd) maxfd = fdTa;
    if (fdIn_BB > maxfd) maxfd = fdIn_BB;

    // Persistent Coordinates (Initialize off-screen or valid default)
    // Removed single coordinates in favor of arrays
                   
    int newH = H - wh;
    int newW = W - ww;

    float x_curr = ww / 2.0;
    float x_prev = ww / 2.0;
    float x_prev2 = ww / 2.0;

    float y_curr = wh / 2.0;
    float y_prev = wh / 2.0;
    float y_prev2 = wh / 2.0;

    // Initial handshake with drone to get starting position
    snprintf(sFromBB, sizeof(sFromBB), "%.0f,%.0f", x_curr, y_curr);
    write(fdFromBB, sFromBB, strlen(sFromBB) + 1);

    if(running == false){
        exit(0);
    }

    while (running) {
        int ch = wgetch(win); // poll window for keys (returns KEY_RESIZE)
        sIn[0]='\0';

        if (ch == KEY_RESIZE) {
            // Store old window dimensions for scaling
            int old_ww = ww;
            int old_wh = wh;

            // Update ncurses internal structures for new dimensions
            resize_term(0, 0);
            layout_and_draw(win);

            // Recalculate and reproportionate obstacles
            for (int i = 0; i < obs_count; i++) {
                mvwprintw(win, obstacles[i].y, obstacles[i].x, " ");
                obstacles[i].x = (int)(((float)obstacles[i].x * ww) / old_ww);
                obstacles[i].y = (int)(((float)obstacles[i].y * wh) / old_wh);
                mvwprintw(win, obstacles[i].y, obstacles[i].x, "O");
                // clamp to window dimensions to prevent vanishing
                if (obstacles[i].x >= ww - 1) obstacles[i].x = ww - 2;
                if (obstacles[i].y >= wh - 1) obstacles[i].y = wh - 2;
            }

            // Recalculate and reproportionate targets
            for (int i = 0; i < tar_count; i++) {
                mvwprintw(win, targets[i].y, targets[i].x, " ");
                targets[i].x = (int)(((float)targets[i].x * ww) / old_ww);
                targets[i].y = (int)(((float)targets[i].y * wh) / old_wh);
                mvwprintw(win, targets[i].y, targets[i].x, "T");
                // clamp to window dimensions to prevent vanishing
                if (targets[i].x >= ww - 1) targets[i].x = ww - 2;
                if (targets[i].y >= wh - 1) targets[i].y = wh - 2;
            }

            // Clamp current position to new window bounds (in case it's off-screen now)
            // Clamp to window dimensions to prevent vanishing
            x_curr = ww / 2;
            y_curr = wh / 2;
            snprintf(sFromBB, sizeof(sFromBB), "%.0f,%.0f", x_curr, y_curr);
            write(fdFromBB, sFromBB, strlen(sFromBB) + 1);
            // 4. SET THE FLAG
            // This tells the code below: "Don't believe the drone for one frame"
            skip_drone_update = true;
        }
        

            // Clamp current position to new window bounds (in case it's off-screen now)
            // Clamp to window dimensions to prevent vanishing
            x_curr = ww / 2;
            y_curr = wh / 2;
            snprintf(sFromBB, sizeof(sFromBB), "%.0f,%.0f", x_curr, y_curr);
            write(fdFromBB, sFromBB, strlen(sFromBB) + 1);
            // 4. SET THE FLAG
            // This tells the code below: "Don't believe the drone for one frame"
            skip_drone_update = true;
        }
        

            // Clear window for new frame
            werase(win);
            box(win, 0, 0);

            FD_ZERO(&readfds);
            FD_SET(fdToBB, &readfds);
            FD_SET(fdOb, &readfds);
            FD_SET(fdTa, &readfds);
            FD_SET(fdIn_BB, &readfds);

            // small timeout so loop stays responsive
            tv.tv_sec = 0;
            tv.tv_usec = 100000;

            retval = select(maxfd + 1, &readfds, NULL, NULL, &tv);

            if (retval == -1) break;
            else if (retval > 0) {
                
                //--- SIMPLIFIED READ INPUT ---
                if (FD_ISSET(fdIn_BB, &readfds)) {
                    // 1. Clear buffer to prevent junk
                    memset(strIn, 0, sizeof(strIn)); 
                    
                    // 2. Read whatever is in the pipe
                    ssize_t bytes = read(fdIn_BB, strIn, sizeof(strIn)-1);
                    
                    if (bytes > 0) {
                        // 3. The Magic Fix: sscanf automatically skips '\n' and ' '
                        // It grabs the first actual letter into sIn
                        sscanf(strIn, "%1s", sIn); 
                    } 
                    else { running = false; } // Pipe closed
                }           

                //....Drone Dynamics...The posistions x, y current
                if (FD_ISSET(fdToBB, &readfds)) {
                    ssize_t bytes = read(fdToBB, sToBB, sizeof(sToBB)-1);
                    if (bytes > 0) {
                        if (skip_drone_update) {
                        // We read the data to clear the pipe, 
                        // BUT we do NOT update x_curr/y_curr.
                        // We just throw this "old" packet away.
                        skip_drone_update = false; // Reset flag for next time
                        } 
                        else {
                            sToBB[bytes] = '\0';
                            sscanf(sToBB, "%f,%f", &x_curr, &y_curr);
                        }   
                    }
                    else { running = false; } // Pipe closed
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

        

        if (sIn[0]=='q'){
            running = false;
        }

         if (sIn[0] == 'a'){
            mvwprintw(win, y_curr, x_curr, " " );
            mvwprintw(win, wh/2,ww/2, "+");
            x_curr=wh/2;
            y_curr=ww/2;

            /* pack all positions into sDrW as: x_curr,y_curr,x_prev,y_prev,x_prev2,y_prev2 */
            snprintf(sFromBB, sizeof(sFromBB), "%.0f,%.0f", x_curr, y_curr);
            //sending new x,y values to drone process        
            write(fdFromBB, sFromBB, strlen(sFromBB) + 1);
            wrefresh(win);
        }

            if (sIn[0] == 'p') {
                mvwprintw(win, 0, 0, "Game Paused, Press 'u' (via pipe) to Resume");
                wrefresh(win);

                fd_set pause_fds;
                struct timeval pause_tv;
                int pause_ret;
                int pause_ch = -1;

                while (running) {
                    // check keyboard
                    pause_ch = getch();
                    sIn[0]='\0';
                    if (pause_ch == 'u') break;

                    // set up select on fds (zero timeout so we poll periodically)
                    FD_ZERO(&pause_fds);
                    FD_SET(fdIn_BB, &pause_fds);
                    FD_SET(fdOb, &pause_fds);
                    FD_SET(fdTa, &pause_fds);
                    pause_tv.tv_sec = 0;
                    pause_tv.tv_usec = 100 * 1000; // 100 ms

                    pause_ret = select(maxfd + 1, &pause_fds, NULL, NULL, &pause_tv);
                    if (pause_ret > 0) {
                        // If input pipe has data, read and update sIn
                        if (FD_ISSET(fdIn_BB, &pause_fds)) {
                            ssize_t bytes = read(fdIn_BB, strIn, sizeof(strIn) - 1);
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
        }

            if (sIn[0] == 'u') {
                // Clear pause message after resume
                mvwprintw(win, 0, 0, "                             ");
                mvwprintw(win, (int)y_curr, (int)x_curr, "+");
                wrefresh(win);
            }

            // 3. WALL COLLISION (No Wrap-Around)
            // If we hit a wall, we clamp the position and reset history 
            // to kill the momentum (otherwise it sticks/vibrates).
            
            if (x_curr >= ww - 1) {
                x_curr = ww - 1;
                x_prev = x_curr; x_prev2 = x_curr; // Stop momentum
                /* pack all positions into sDrW as: x_curr,y_curr,x_prev,y_prev,x_prev2,y_prev2 */
                snprintf(sFromBB, sizeof(sFromBB), "%.0f,%.0f", x_curr, y_curr);
                //sending new x,y values to drone process        
                write(fdFromBB, sFromBB, strlen(sFromBB) + 1);
            } else if (x_curr <= 0) {
                x_curr = 0;
                //x_prev = x_curr; x_prev2 = x_curr; // Stop momentum
                /* pack all positions into sDrW as: x_curr,y_curr,x_prev,y_prev,x_prev2,y_prev2 */
                snprintf(sFromBB, sizeof(sFromBB), "%.0f,%.0f", x_curr, y_curr);
                //sending new x,y values to drone process        
                write(fdFromBB, sFromBB, strlen(sFromBB) + 1);
            }

            if (y_curr >= wh - 1) {
                y_curr = wh - 1;
                //y_prev = y_curr; y_prev2 = y_curr; // Stop momentum
                /* pack all positions into sDrW as: x_curr,y_curr,x_prev,y_prev,x_prev2,y_prev2 */
                snprintf(sFromBB, sizeof(sFromBB), "%.0f,%.0f", x_curr, y_curr);
                //sending new x,y values to drone process        
                write(fdFromBB, sFromBB, strlen(sFromBB) + 1);
                
            } else if (y_curr <= 0) {
                y_curr = 0;
                //y_prev = y_curr; y_prev2 = y_curr; // Stop momentum
                /* pack all positions into sDrW as: x_curr,y_curr,x_prev,y_prev,x_prev2,y_prev2 */
                snprintf(sFromBB, sizeof(sFromBB), "%.0f,%.0f", x_curr, y_curr);
                //sending new x,y values to drone process        
                write(fdFromBB, sFromBB, strlen(sFromBB) + 1);
                
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
        
        // --- 6. REFRESH SCREEN ---
        wrefresh(win);

        // --- 7. FRAME DELAY ---
        usleep(1000); 
        
    }
    // Cleanup
    delwin(win);
    endwin();
    return 0;

}





