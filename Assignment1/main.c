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
    int fdIn[2];
 
    char buf[100];
    int toggle = 0;


 //check if pipes initialize
    if (pipe(fdIn) == -1) {
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

        // Execute process_P with fd[1] as a command-line argument
        char fdIn_str[10];
        snprintf(fdIn_str, sizeof(fdIn_str), "%d", fdIn[0]);
        
        execlp("konsole", "konsole", "-e", "./BlackBoard", fdIn_str, (char *)NULL); // launch another process if condition met
       
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

    wait(NULL);

}