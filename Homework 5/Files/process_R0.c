#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
  
int main(int argc, char *argv[]) 
{

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fd>\n", argv[0]);
        exit(1);
    }

   //convert the argument to an integer file descriptor
    int fdR0 = atoi(argv[1]); //reading end
    char buffer[100];
    char format_string[100]= "%s";
    char s1[100];
    char log_history[4096]="";
    int log_size=0;

     while (1) 
    {
                
           
     ssize_t bytesB= read(fdR0, buffer, sizeof(buffer)-1); 

        if (bytesB <= 0) 
        {
            printf("Pipe closed\n");
            close(fdR0);
            break;
        }

        buffer[bytesB]='\0';
        
        /* read numbers from input line */
        sscanf(buffer, format_string, &s1 );

        printf("Read from BB : %s", s1);
    
       if (log_size + strlen(s1) + 1 < 4096) 
        {
            strcat(log_history, s1); // Append the new string
            strcat(log_history, "\n"); // Add a newline for readability
            log_size += strlen(s1) + 1;
        }

        printf("\n--- FINAL LOG HISTORY ---\n%s\n", log_history);
        fflush(stdout);

    
    } 

     
}