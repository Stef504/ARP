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
#include <errno.h>


int main()
{
    int fdIn[2], fdOb[2], fdTa[2],fdToBB[2], fdFromBB[2],fdRepul[2];

    const char * pipe_path = "./pipe_blackboard_input";
    
    // Remove existing pipe if it exists (ignore error if it doesn't exist)
    if (unlink(pipe_path) == -1 && errno != ENOENT) {
        perror("Warning: Failed to remove existing pipe");
    }
 

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

    if (pipe(fdToBB) == -1) {
        perror("pipe failed");
        exit(1);
    } 

    if (pipe(fdFromBB) == -1) {
        perror("pipe failed");
        exit(1);
    } 
    
    if (pipe(fdRepul) == -1) {
        perror("pipe failed");
        exit(1);
    }
    
    if (mkfifo(pipe_path, 0666) == -1) {
        perror("Failed to create named pipe");
        exit(1);
    }

    sleep(2);

    //.....BlackBoard.....
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

        //So fdDrR - close the writing because we read the dynamics 
        close(fdToBB[1]);
        //so fdDrW - close the reading because we if there is an Ob/Ta
        close(fdFromBB[0]);

        //close ob and ta writing ends in bb
        close(fdOb[1]);
        close(fdTa[1]);

        //close fdRepul read end
        close(fdRepul[0]);

        // Execute process_P with fd[1] as a command-line argument
        char fdOb_str[10];
        snprintf(fdOb_str, sizeof(fdOb_str), "%d", fdOb[0]);
        char fdTa_str[10];
        snprintf(fdTa_str, sizeof(fdTa_str), "%d", fdTa[0]);

        //drone pipes
        char fdToBB_str[10];
        snprintf(fdToBB_str, sizeof(fdToBB_str), "%d", fdToBB[0]);

        char fdFromBB_str[10];
        snprintf(fdFromBB_str, sizeof(fdFromBB_str), "%d", fdFromBB[1]);

        char fdRepul_str[10];
        snprintf(fdRepul_str,sizeof(fdRepul),"%d",fdRepul[1]);
        
        
        execlp("konsole", "konsole", "-e", "./BlackBoard",fdToBB_str,fdFromBB_str,fdOb_str,fdTa_str,"./pipe_blackboard_input",fdRepul_str, (char *)NULL); // launch another process if condition met
       
        // If exec fails
        perror("exec failed");
        exit(1);
     

    }

    //.....Input.....
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


        // Convert fd[1] to a string to pass as an argument, fd[1] is for writing
        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", fdIn[1]);//saying whatever it reads store in fd_str


        // Execute process_P with fd[1] as a command-line argument
        
        execlp("konsole", "konsole", "-e", "./process_In", fd_str, "./pipe_blackboard_input",(char *)NULL); // launch another process if condition met
       
        perror("exec failed");
        exit(1);
     

    }

    
    //.....Obstacle.....
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


    //.....Targets.....
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
        
        execlp("./process_Ta", "./process_Ta", fd_str, (char *)NULL); // launch another process if condition met
       
        // If exec fails
        perror("exec failed");
        exit(1);
    
    }

    //.....Drone.....
    pid_t Dr=fork();

    if (Dr < 0)
    {
    perror("Error in fork");
    return 1;
    }

    if (Dr == 0)
    {
       
        // Child process
        printf("Process Ta: PID = %d\n", getpid()); //getpid gets the file id

        // Close the read end, as we write to bb
        close(fdToBB[0]);

        //close the write end, as we read from bb
        close(fdFromBB[1]);

        // Close the writing end of the pipe in the child
        close(fdIn[1]);

        //close fdRepul write end
        close(fdRepul[1]);

        // Convert fd[1] to a string to pass as an argument, fd[1] is for writing
        char fdtoBB_str[10];
        snprintf(fdtoBB_str, sizeof(fdtoBB_str), "%d", fdToBB[1]);//saying whatever it reads store in fd_str

        char fdFromBB_str[10];
        snprintf(fdFromBB_str, sizeof(fdFromBB_str), "%d", fdFromBB[0]);//saying whatever it reads store in fd_str

        char fdIn_str[10];
        snprintf(fdIn_str, sizeof(fdIn_str), "%d", fdIn[0]);
        // Execute process_P with fd[1] as a command-line argument

        char fdRepul_str[10];
        snprintf(fdRepul_str,sizeof(fdRepul_str),"%d",fdRepul[0]);
        
        execlp("./process_Drone", "./process_Drone",fdIn_str,fdFromBB_str,fdtoBB_str,fdRepul_str, (char *)NULL); // launch another process if condition met
       
        // If exec fails
        perror("exec failed");
        exit(1);
    
    }

    //closing all pipes
    // Close Input Pipes
    close(fdIn[0]); close(fdIn[1]);

    // Close Obstacle Pipes
    close(fdOb[0]); close(fdOb[1]);

    // Close Target Pipes
    close(fdTa[0]); close(fdTa[1]);

    // Close Blackboard communication Pipes
    close(fdToBB[0]); close(fdToBB[1]);
    close(fdFromBB[0]); close(fdFromBB[1]);

    // Close Repulsion Pipes
    close(fdRepul[0]); close(fdRepul[1]);

    // Wait for all child processes with status checking
    int status;
    int failures = 0;
    pid_t wpid;

    //checking if the child status has changed
    while ((wpid = wait(&status)) > 0) {

        //how child terminated, reads from status. Either normally exit()/ return or code
        if (WIFEXITED(status)) { 
            int code = WEXITSTATUS(status);
            
            // Check if this was the drone process
            if (wpid == Dr) {
                fprintf(stderr, "Drone exited, shutting down all processes...\n");
                
                // Kill all other child processes
                kill(BB, SIGTERM);
                kill(In, SIGTERM);
                kill(Ob, SIGTERM);
                kill(Ta,SIGTERM);
                
                // Small delay to let them terminate gracefully
                usleep(100000);
                
                // Force kill if still alive
                kill(BB, SIGKILL);
                kill(In, SIGKILL);
                kill(Ob, SIGKILL);
                kill(Ta,SIGKILL);
                
                break; // Exit the wait loop
            }
            //failures
            if (code != 0) {
                fprintf(stderr, "Child %d exited with code %d\n", wpid, code);
                failures++;
            }
            //process was terminated through signals
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "Child %d terminated by signal %d\n", wpid, WTERMSIG(status));
            failures++;
        }
    }

    // Wait for remaining children to finish
    while (wait(NULL) > 0);

    if (failures) {
        fprintf(stderr, "One or more children failed (%d)\n", failures);
    }

    //unlink the named pipe
    unlink(pipe_path);

    return 0;
}