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

int main(int argc, char *argv[]) 
{

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fd>\n", argv[0]);
        exit(1);
    }

    //convert the argument to an integer file descriptor
    int fdOb = atoi(argv[1]);
    char buffer[100];

    //generate random coordinates 

    FILE* file = fopen("Parameter_File.txt", "r"); 
    char line[256]; 
    int i = 0; 
    while (fgets(line, sizeof(line), file)) { 
        i++; 
        if (i == 1){
            char* string1[] = delimit(line, "_");
        }
        if (i == 2){
            char* string2[] = delimit(line, "_");
        }

    } 
    fclose(file);

    while(1){
        //write it to pipe
        x_coord = rand() % (0, string1[2]);
        y_coord = rand() % (0, string2[2]);

        sprintf(buffer, "%d,%d", x_coord, y_coord);
        write(fdOb, buffer, strlen(buffer)+1);
        sleep(5);
    }
 
}