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
#include <time.h>

int window_width;
int window_height;
int x_coord_Ob;
int y_coord_Ob; 

static long current_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000L + tv.tv_usec / 1000L;
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
        }
    }
    fclose(file);
}



int main(int argc, char *argv[]) 
{
    Parameter_File();

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fd>\n", argv[0]);
        exit(1);
    }

    srand(time(NULL) + getpid());

    //convert the argument to an integer file descriptor
    int fdOb = atoi(argv[1]);
    char buffer[100];

    const long obstacle_interval_ms = 5000;
    long last_obstacle_ms = current_millis();

    while(1){
        //write it to pipe
        sleep(1);
        long now_ms = current_millis();
        if (now_ms - last_obstacle_ms >= obstacle_interval_ms) {
            srand(time(NULL));
            x_coord_Ob = 1 + rand() % (window_width - 10);
            y_coord_Ob = 1 + rand() % (window_height - 10);
            last_obstacle_ms = now_ms;
            }
        sprintf(buffer, "%d,%d", x_coord_Ob, y_coord_Ob);
        write(fdOb, buffer, strlen(buffer)+1);

    }
 
}
            
            
    