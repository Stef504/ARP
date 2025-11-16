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

int window_width;
int window_height;

int main()
{
    int fdIn[2], fdOb[2], fdTa[2];
 
    char buf[100];
    int toggle = 0;

    FILE* file = fopen("Parameter_File.txt", "r"); 
    char line[256]; 
    int i = 0; 
    while (fgets(line, sizeof(line), file)) { 
        i++; 
        if (i == 1){
            char* string1[] = strtok(line, "_");
        }
        if (i == 2){
            char* string2[] = strtok(line, "_");
        }

    } 
    fclose(file);
    window_width = atoi(string1[2]);
    window_height = atoi(string2[2]);

 //check if pipes initialize
    if (pipe(fdIn) == -1) {
        perror("pipe failed");
        exit(1);
    }  

        if (pipe(fdOb) == -1) {
        perror("pipe failed");
        exit(1);
    } 

        if (pipe(fdTa) == -1) {
        perror("pipe failed");
        exit(1);
    } 

    sleep(2);

    pid_t BB=fork();
    if (BB < 0)
   {
    perror("Error in fork");
    return 1;
    }

    if (BB == 0)
    {
       
        // Child process
        printf("Process BB: PID = %d\n", getpid()); //getpid gets the file id

        //this reads so close writing end
        close(fdIn[1]);
        close(fdOb[1]);
        close(fdTa[1]);

        // Execute process_P with fd[1] as a command-line argument
        char fdIn_str[10];
        snprintf(fdIn_str, sizeof(fdIn_str), "%d", fdIn[0]);
        
        char fdOb_str[10];
        snprintf(fdOb_str, sizeof(fdOb_str), "%d", fdOb[0]);
        
        char fdTa_str[10];
        snprintf(fdTa_str, sizeof(fdTa_str), "%d", fdTa[0]);
                

        
        execlp("konsole", "konsole", "-e", "./BlackBoard", fdIn_str,fdOb_str, fdTa_str, (char *)NULL); // launch another process if condition met
       
        // If exec fails
        perror("exec failed");
        exit(1);
     

    }

    pid_t In=fork();

        if (In < 0)
   {
    perror("Error in fork");
    return 1;
    }

    if (In == 0)
    {
       
        // Child process
        printf("Process In: PID = %d\n", getpid()); //getpid gets the file id

        // Close the reading end of the pipe in the child
        close(fdIn[0]);

        // Convert fd[1] to a string to pass as an argument, fd[1] is for writing
        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", fdIn[1]);//saying whatever it reads store in fd_str

        // Execute process_P with fd[1] as a command-line argument
        
        execlp("konsole", "konsole", "-e", "./process_In", fd_str, (char *)NULL); // launch another process if condition met
       
        // If exec fails
        perror("exec failed");
        exit(1);
     

    }

        pid_t Ob=fork();

        if (Ob < 0)
   {
    perror("Error in fork");
    return 1;
    }

    if (Ob == 0)
    {
       
        // Child process
        printf("Process Ob: PID = %d\n", getpid()); //getpid gets the file id

        // Close the reading end of the pipe in the child
        close(fdOb[0]);

        // Convert fd[1] to a string to pass as an argument, fd[1] is for writing
        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", fdOb[1]);//saying whatever it reads store in fd_str

        // Execute process_P with fd[1] as a command-line argument
        
        execvp("./process_Ob", fd_str); // launch another process if condition met
       
        // If exec fails
        perror("exec failed");
        exit(1);
     

    }

    pid_t Ta=fork();

        if (Ta < 0)
   {
    perror("Error in fork");
    return 1;
    }

    if (Ta == 0)
    {
       
        // Child process
        printf("Process Ta: PID = %d\n", getpid()); //getpid gets the file id

        // Close the reading end of the pipe in the child
        close(fdTa[0]);

        // Convert fd[1] to a string to pass as an argument, fd[1] is for writing
        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", fdTa[1]);//saying whatever it reads store in fd_str

        // Execute process_P with fd[1] as a command-line argument
        
        execvp("./process_Ta", fd_str); // launch another process if condition met
       
        // If exec fails
        perror("exec failed");
        exit(1);
     

    }

    wait(NULL);

}