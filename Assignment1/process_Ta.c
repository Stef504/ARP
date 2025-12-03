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
int x_coord_Ta, y_coord_Ta;

static long current_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000L + tv.tv_usec / 1000L;
}

// Same function as in BlackBoard, but read only the first two lines of the parameter file
void Parameter_File() {
    // Standardized exit codes
    #define USAGE_ERROR 64
    #define OPEN_FAIL 66
    #define EXEC_FAIL 127
    #define RUNTIME_ERROR 70

    FILE* file = fopen("Parameter_File.txt", "r");
    if (file == NULL) {
        perror("Error opening Parameter_File.txt");
        exit(OPEN_FAIL);
    }

    char line[256];
    int line_number = 0;

    while (fgets(line, sizeof(line), file)) {
        line_number++;

        char* tokens[10]; 
        int token_count = 0;
        char* token = strtok(line, "_");

        while (token != NULL && token_count < 10) {
            tokens[token_count] = token; 
            token_count++;
            token = strtok(NULL, "_"); 
        }

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
        exit(USAGE_ERROR);
    }
    
    // Convert the argument to an integer file descriptor
    int fdTa = atoi(argv[1]);
    char buffer[100];

    // Target generation every 7 seconds
    const long target_interval_ms = 7000; 
    long last_target_ms = current_millis();
    srand(time(NULL) + getpid());

    while(1){
        long now_ms = current_millis();
        if (now_ms - last_target_ms >= target_interval_ms) {
            x_coord_Ta = 1 + rand() % (window_width - 10);
            y_coord_Ta = 1 + rand() % (window_height - 10);
            last_target_ms = now_ms;
            sprintf(buffer, "%d,%d", x_coord_Ta, y_coord_Ta);
            write(fdTa, buffer, strlen(buffer)+1);
        }
        usleep(100000); // Sleep 100ms to avoid busy-waiting
    }

    //clean up
    close(fdTa);

    return 0;
}