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
    int fdIn[2], fdOb[2], fdTa[2];
 
    char buf[100];
    int toggle = 0;


 //check if pipes initialize
    if (pipe(fdIn) == -1) {
        perror("pipe fdIn failed");
        exit(1);
    }
    
    if (pipe(fdOb) == -1) {
        perror("pipe fdOb failed");
        exit(1);
    }
    
    if (pipe(fdTa) == -1) {
        perror("pipe fdTa failed");
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
        printf("Process BB: PID = %d\n", getpid());

        // Closing writing end from pipes linked to main 
        close(fdIn[1]);
        close(fdOb[1]);
        close(fdTa[1]);

        // Execute process_P with fd[1] as a command-line argument
        char fdIn_str[10], fdOb_str[10], fdTa_str[10];

        snprintf(fdIn_str, sizeof(fdIn_str), "%d", fdIn[0]);
        snprintf(fdOb_str, sizeof(fdOb_str), "%d", fdOb[0]);
        snprintf(fdTa_str, sizeof(fdTa_str), "%d", fdTa[0]);
        
        execlp("konsole", "konsole", "-e", "./BlackBoard", fdIn_str,fdOb_str,fdTa_str, (char *)NULL);
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
       
        printf("Process In: PID = %d\n", getpid()); 

        close(fdIn[0]);

        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", fdIn[1]);
        
        execlp("konsole", "konsole", "-e", "./process_In", fd_str, (char *)NULL); 
       
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
       
        printf("Process Ob: PID = %d\n", getpid()); 

        close(fdOb[0]);

        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", fdOb[1]);
        
        execlp("./process_Ob", "./process_Ob", fd_str, (char *)NULL);
       
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
       
        printf("Process Ta: PID = %d\n", getpid()); 

        close(fdTa[0]);

        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", fdTa[1]);
        
        execlp("./process_Ta", "./process_Ta", fd_str, (char *)NULL); 
       
        perror("exec failed");
        exit(1);
    
    }

    // Wait for all child processes
    wait(NULL);
    wait(NULL);
    wait(NULL);
    wait(NULL);

    return 0;
}