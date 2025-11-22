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
#include <signal.h>
#include <errno.h>

int window_width ;
int window_height ;
int rph_intial ;
double eta_intial;
int force_intial ;
int mass;        // sensible default
int k_intial;
int working_area ;
int t_intial ;  // safe default timeout (ms)
bool running = true;

// --- HELPER: Identify Opposite Keys ---
char get_opposite_key(char key) {
    switch (key) {
        case 'w': return 'v'; // Up-Left  vs Down-Right
        case 'e': return 'c'; // Up       vs Down
        case 'r': return 'x'; // Up-Right vs Down-Left
        case 's': return 'f'; // Left     vs Right
        case 'f': return 's'; // Right    vs Left
        case 'x': return 'r'; // Down-Left vs Up-Right
        case 'c': return 'e'; // Down     vs Up
        case 'v': return 'w'; // Down-Right vs Up-Left
        default: return 0;
    }
}

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

int main(int argc, char *argv[]) 
{
    Parameter_File();

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <fd>\n", argv[0]);
        exit(1);
    }

    //convert the argument to an integer file descriptor
    int fdIn = atoi(argv[1]);
    int fdFromBB= atoi(argv[2]);
    int fdToBB = atoi(argv[3]);

    // Avoid process termination on broken pipe and print FD debug
    signal(SIGPIPE, SIG_IGN);
    dprintf(STDERR_FILENO, "DRONE: start fds fdIn=%d fdFromBB=%d fdToBB=%d\n",
            fdIn, fdFromBB, fdToBB);
    
    struct timeval tv={0,0};
    int retval;
    char strIn[135],sOut[135],strFromBB[100]; 
    char sIn[10];

    int x_coord_Ob, y_coord_Ob;
    int x_coord_Ta, y_coord_Ta;

    // Physics Variables
    float x_curr = 0, y_curr = 0;
    float x_prev = 0, y_prev = 0;
    float x_prev2 = 0, y_prev2 = 0;
    float x_update =0 , y_update =0;

    // ---------------------------------------------------------
    // HANDSHAKE: Wait for Blackboard to say where to start
    // ---------------------------------------------------------
    
    // 1. BLOCKING READ: The program STOPS here until Blackboard writes
    ssize_t bytes = 0;
    while (bytes <= 0) {
        bytes = read(fdFromBB, strFromBB, sizeof(strFromBB)-1);
    }
    strFromBB[bytes] = '\0';

    // 2. Parse the Center Coordinates
    sscanf(strFromBB, "%f,%f",&x_curr, &y_curr);
    
    x_prev = x_curr;
    x_prev2 = x_curr;
    y_prev = y_curr;
    y_prev2 = y_curr;

    fd_set readfds;

    int maxfd = fdIn;
    if (fdFromBB > maxfd) maxfd = fdFromBB;
 
    float diag_force = (float)force_intial * M_SQRT1_2;
    float T= t_intial / 1000.0; // Convert ms to seconds

    // --- MOMENTUM STATE VARIABLES ---
    char active_key = ' '; // The key currently driving the physics
    int boost_level = 0;   // 0 = 0%, 1 = 10%, 2 = 20% (Max)

    while(running){
 
        FD_ZERO(&readfds);
        FD_SET(fdIn, &readfds);
        FD_SET(fdFromBB, &readfds);

        // FIX: Reset timer every loop
        tv.tv_sec = 0;
        tv.tv_usec = 0; // Use a zero-timeout for select to poll

        // 2. WAIT FOR INPUT / TIMER
        retval = select(maxfd + 1, &readfds, NULL, NULL, &tv);

        if (retval == -1) {
            break;
        } 
        else if (retval > 0) {
            // --- INPUT PIPE ---
            if (FD_ISSET(fdIn, &readfds)) {
                ssize_t bytes = read(fdIn, strIn, sizeof(strIn)-1);
                if (bytes > 0) {
                    strIn[bytes] = '\0';
                    sscanf(strIn, "%s", sIn);
                } else { running = false; } // Pipe closed
            }

            // --- Read from black board PIPE ---
            if (FD_ISSET(fdFromBB, &readfds)) {
                ssize_t bytes = read(fdFromBB, strFromBB, sizeof(strFromBB)-1);
                if (bytes > 0) {
                    strFromBB[bytes] = '\0';
                    // reading our x,y positions from bb only when there is something to read
                    //so like resize,a..
                    sscanf(strFromBB, "%f,%f",&x_update, &y_update);
                    // RESET PHYSICS HISTORY
                    x_prev = x_update;
                    x_prev2 = x_update;
                    y_prev = y_update;
                    y_prev2 = y_update;
                }
            }
        }                
        
          // --- SEQUENCE LOGIC ---
        char input_key = sIn[0];
        
        // Only process if we received a real command (not empty space)
        if (input_key != ' ' && input_key != 0) {
            
            // Case A: Quit
            if (input_key == 'q') running = false;

            // Case B: Brake (Stop Engine)
            else if (input_key == 'd') {
                boost_level = 0;
                active_key = ' ';
            }
            // Case C: Pause Logic
            else if (input_key == 'p') {
                // Enter Blocking Wait for 'u'
                while(1) {
                    ssize_t b = 0;
                    while (b <= 0) b = read(fdIn, strIn, sizeof(strIn)-1);
                    strIn[b] = '\0';
                    sscanf(strIn, "%s", sIn);
                    if (sIn[0] == 'u') break;
                }
            }
            // Case D: Reset Logic
            else if (input_key == 'a') {
                // Read Reset Position from Blackboard
                ssize_t b = 0;
                while (b <= 0) b = read(fdFromBB, strFromBB, sizeof(strFromBB)-1);
                strFromBB[b] = '\0';
                sscanf(strFromBB, "%f,%f", &x_update, &y_update);
                
                // Reset Physics State
                x_prev = x_update; x_prev2 = x_update;
                y_prev = y_update; y_prev2 = y_update;
                boost_level = 0; active_key = ' ';
            }
            // Case E: Same Direction -> Increase Speed
            else if (input_key == active_key) {
                if (boost_level < 2) boost_level++; 
            }
            // Case F: Opposite Direction -> Decrease Speed
            else if (input_key == get_opposite_key(active_key)) {
                boost_level--;
                if (boost_level < 0) {
                    // Crossed the threshold: Reverse Direction
                    boost_level = 0;
                    active_key = input_key;
                }
            }
            // Case G: New Direction (Orthogonal) -> Switch immediately
            else {
                boost_level = 0;
                active_key = input_key;
            }
        }
        
        // Clear physical input so keys don't "stick" in the buffer,
        // BUT active_key persists, so the engine keeps running.
        sIn[0] = ' '; 

        // --- CALCULATE FORCE ---
        float multiplier = 1.0 + (boost_level * 0.5);
        float cur_force = force_intial * multiplier;
        float cur_diag = diag_force * multiplier;

        // --- APPLY FORCE (Based on ACTIVE KEY, not just input) ---
        float Fx = 0, Fy = 0;

         switch (active_key) {
            case 'e': Fy = -cur_force; break; // Up
            case 'c': Fy =  cur_force; break; // Down
            case 's': Fx = -cur_force; break; // Left
            case 'f': Fx =  cur_force; break; // Right
            case 'w': Fx = -cur_diag; Fy = -cur_diag; break;
            case 'r': Fx =  cur_diag; Fy = -cur_diag; break;
            case 'x': Fx = -cur_diag; Fy =  cur_diag; break;
            case 'v': Fx =  cur_diag; Fy =  cur_diag; break;
        }
    //.....Drone..... 
    // 2. PHYSICS CALCULATION (The Equation)
    // We calculate X and Y independently to allow 8-direction movement.

    
      // --- PHYSICS INTEGRATION (Euler) ---
    float denom = mass + (k_intial * T);
    float history_factor = (2 * mass) + (k_intial * T);

    float num_x = (Fx * T * T) + (x_prev * history_factor) - (mass * x_prev2);
    float x_new = num_x / denom;

    float num_y = (Fy * T * T) + (y_prev * history_factor) - (mass * y_prev2);
    float y_new = num_y / denom;
    
     // --- UPDATE HISTORY ---
    x_prev2 = x_prev; x_prev = x_new;
    y_prev2 = y_prev; y_prev = y_new;
    x_curr = x_new;
    y_curr = y_new;
    
    //sends back the current position to bb
    snprintf(sOut, sizeof(sOut), "%d,%d", (int)(x_curr), (int)(y_curr));
    ssize_t w = write(fdToBB, sOut, strlen(sOut) + 1);
    if (w > 0) {
        dprintf(STDERR_FILENO, "DRONE: wrote %zd bytes to fd %d -> '%s'\n", w, fdToBB, sOut);
    } else {
        dprintf(STDERR_FILENO, "DRONE: write failed fd=%d ret=%zd errno=%d (%s)\n",
                fdToBB, w, errno, strerror(errno));
    }
     
    usleep(10000);
}
}