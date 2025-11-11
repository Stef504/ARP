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

int main()
{
    int fdW0[2], fdW1[2], fdR0[2], fdR1[2]; 
 
    char buf[100];
    int toggle = 0;



 //check if pipes initialize
    if (pipe(fdW0) == -1) {
        perror("pipe failed");
        exit(1);
    } 

    if (pipe(fdW1) == -1) {
        perror("pipe failed");
        exit(1);
    } 

        if (pipe(fdR0) == -1) {
        perror("pipe failed");
        exit(1);
    } 

    if (pipe(fdR1) == -1) {
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
        close(fdW0[1]);
        close(fdW1[1]);

        //this writes to close reading end
        close(fdR0[0]);
        close(fdR1[0]);

        // Execute process_P with fd[1] as a command-line argument
        char fdW0_str[10];
        snprintf(fdW0_str, sizeof(fdW0_str), "%d", fdW0[0]);
        
        char fdW1_str[10];
        snprintf(fdW1_str, sizeof(fdW1_str), "%d", fdW1[0]);
        
        char fdR0_str[10];
        snprintf(fdR0_str, sizeof(fdR0_str), "%d", fdR0[1]);
        
        char fdR1_str[10];
        snprintf(fdR1_str, sizeof(fdR1_str), "%d", fdR1[1]);
        

        
        execlp("konsole", "konsole", "-e", "./process_BB", fdW0_str,fdW1_str, fdR0_str,fdR1_str, (char *)NULL); // launch another process if condition met
       
        // If exec fails
        perror("exec failed");
        exit(1);
     

    }

    pid_t W0=fork();

        if (W0 < 0)
   {
    perror("Error in fork");
    return 1;
    }

    if (W0 == 0)
    {
       
        // Child process
        printf("Process W0: PID = %d\n", getpid()); //getpid gets the file id

        // Close the reading end of the pipe in the child
        close(fdW0[0]);

        // Convert fd[1] to a string to pass as an argument, fd[1] is for writing
        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", fdW0[1]);//saying whatever it reads store in fd_str

        // Execute process_P with fd[1] as a command-line argument
        
        execlp("konsole", "konsole", "-e", "./process_W0", fd_str, (char *)NULL); // launch another process if condition met
       
        // If exec fails
        perror("exec failed");
        exit(1);
     

    }

        pid_t W1=fork();

        if (W1 < 0)
   {
    perror("Error in fork");
    return 1;
    }

    if (W1 == 0)
    {
       
        // Child process
        printf("Process W1: PID = %d\n", getpid()); //getpid gets the file id

        // Close the reading end of the pipe in the child
        close(fdW1[0]);

        // Convert fd[1] to a string to pass as an argument, fd[1] is for writing
        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", fdW1[1]);//saying whatever it reads store in fd_str

        // Execute process_P with fd[1] as a command-line argument
        
        execlp("konsole", "konsole", "-e", "./process_W1", fd_str, (char *)NULL); // launch another process if condition met
       
        // If exec fails
        perror("exec failed");
        exit(1);
     

    }

    pid_t R0=fork();

        if (R0 < 0)
   {
    perror("Error in fork");
    return 1;
    }

    if (R0 == 0)
    {
       
        // Child process
        printf("Process R0: PID = %d\n", getpid()); //getpid gets the file id

        // Close the writing end of the pipe in the child
        close(fdR0[1]);

        // Convert fd[1] to a string to pass as an argument, fd[1] is for writing
        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", fdR0[0]);//saying whatever it reads store in fd_str

        // Execute process_P with fd[1] as a command-line argument
        
        execlp("konsole", "konsole", "-e", "./process_R0", fd_str, (char *)NULL); // launch another process if condition met
       
        // If exec fails
        perror("exec failed");
        exit(1);
     

    }

        pid_t R1=fork();

        if (R1 < 0)
   {
    perror("Error in fork");
    return 1;
    }

    if (R1 == 0)
    {
       
        // Child process
        printf("Process R1: PID = %d\n", getpid()); //getpid gets the file id

        // Close the writing end of the pipe in the child
        close(fdR1[1]);

        // Convert fd[1] to a string to pass as an argument, fd[1] is for writing
        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", fdR1[0]);//saying whatever it reads store in fd_str

        // Execute process_P with fd[1] as a command-line argument
        
        execlp("konsole", "konsole", "-e", "./process_R1", fd_str, (char *)NULL); // launch another process if condition met
       
        // If exec fails
        perror("exec failed");
        exit(1);
     

    }


}