#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#define _XOPEN_SOURCE_EXTENDED
#include <locale.h>
#include <ncurses.h>
  
int main(int argc, char *argv[]) {

    if (argc < 3) 
    {
        fprintf(stderr, "Usage: %s <fd>\n", argv[0]);
        exit(1);
    }

    // Convert the argument to an integer file descriptor
    //order from main fdW0_str,fdW1_str, fdR0_str,fdR1_str
    int fdIn = atoi(argv[1]);
    char bufferIn[100];

    int fdob = atoi(argv[2]);
    char bufferOb[100];

    int fdTa = atoi(argv[3]);
    char bufferTa[100];



    struct timeval tv; //name of timeout tv
    int retval,counts1,counts2,countw1,countw2 ;
    char strIn[100], strOb[100],strTa[100],strR0[100];
    char s1[100],s2[100],s3[100],s4[100];
    char format_stringIn[100]="%s";
    char format_stringOb[100]="%d,%d";
    char format_stringTa[100]="%d,%d";

    fd_set readfds;

    int maxfd = fdIn;
    if (fdOb > maxfd) maxfd = fdOb;
    if (fdTa > maxfd) maxfd = fdTa;

    //TO DO:
    //read from input : chars
    //let ncurses read the input and do the action
    //Let ncurses read the input from Ob, Ta
    //calculate the force and let ncurses respond accordingly

    while (1){
        FD_ZERO(&readfds);
        FD_SET(fdIn, &readfds);
        FD_SET(fdOb, &readfds);
        FD_SET(fdTa, &readfds);

        /* Wait up to five seconds. */
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        retval = select(maxfd + 1, &readfds, NULL, NULL, &tv); //here is select, second slot is for
                                                        //for write checks
 
        /* Don’t rely on the value of tv now! */
        if (retval == -1)
            perror("select()");
            else if (retval > 0) {
                    // Data from input pipeline
                    if (FD_ISSET(fdIn, &readfds)) 
                    {
                        ssize_t bytesIn = read(fdIn, strIn, sizeof(strIn)-1); 
                        
                     if (bytesIn <= 0) 
                        {
                            printf("Pipe closed\n");
                            break;
                        }

                     strIn[bytesIn]='\0';
        
                     /* read numbers from input line */
                     sscanf(strIn, format_stringIn, s1);

                    } 

                    // Data from Ob pipeline
                    if (FD_ISSET(fdOb, &readfds)) 
                    {
                        ssize_t bytesOb = read(fdOb, strOb, sizeof(strOb)-1); 
                        

                     if (bytesOb <= 0) 
                        {
                            printf("Pipe closed\n");
                            break;
                        }

                     strOb[bytesOb]='\0';
        
                     /* read numbers from input line */
                     sscanf(strOb, format_stringOb, s2);
                    
                    }

                    // Data from Ta pipeline
                    if (FD_ISSET(fdTa, &readfds)) 
                    {
                        ssize_t bytesTa = read(fdTa, strTa, sizeof(strTa)-1); 
                        

                     if (bytesTa <= 0) 
                        {
                            printf("Pipe closed\n");
                            break;
                        }

                     strTa[bytesTa]='\0';
        
                     /* read numbers from input line */
                     sscanf(strTa, format_stringTa, s3);
                    
                    }
                    
                    
                }
            
    }
        
}
    
    


 
 