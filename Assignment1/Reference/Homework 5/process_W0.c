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
    int fdW0 = atoi(argv[1]);
    char buffer[100];
    

     while (1) 
 {
                
        printf("A> ");
        
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) 
    {
        write(fdW0, buffer, strlen(buffer)+1);
        
    }  

    if (buffer[0] == 'q')
    { 
            close(fdW0);
            exit(EXIT_SUCCESS);
            
    }
    
 } 

     

}