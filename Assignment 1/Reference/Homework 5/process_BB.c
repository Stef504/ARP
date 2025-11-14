#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
  
int main(int argc, char *argv[]) 
{
    if (argc < 5) 
    {
        fprintf(stderr, "Usage: %s <fd>\n", argv[0]);
        exit(1);
    }

    // Convert the argument to an integer file descriptor
    //order from main fdW0_str,fdW1_str, fdR0_str,fdR1_str
    int fdW0 = atoi(argv[1]);
    char bufferW0[100];

    int fdW1 = atoi(argv[2]);
    char bufferW1[100];

    int fdR0 = atoi(argv[3]);
    char bufferR0[100];

    int fdR1 = atoi(argv[4]);
    char bufferR1[100];


    struct timeval tv; //name of timeout tv
    int retval,counts1,counts2,countw1,countw2 ;
    char strW0[100], strW1[100],strR1[100],strR0[100];
    char s1[100],s2[100],s3[100],s4[100];
    char format_stringW0[100]="%s";
    char format_stringW1[100]="%s";
    char format_stringR0[100]="%s";
    char format_stringR1[100]="%s";
    
    char old_s1[100] = "%s"; // Storage for W0's last value
    char old_s2[100] = "%s"; // Storage for W1's last value

    fd_set readfds;
    fd_set writefds;

    int maxfd = fdW0;
    if (fdW1 > maxfd) maxfd = fdW1;
    if (fdR0 > maxfd) maxfd = fdR0;
    if (fdR1 > maxfd) maxfd = fdR1;

    counts1=-1;
    counts2=-1;

    countw1=0;
    countw2=0;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(fdW0, &readfds);
        FD_SET(fdW1, &readfds);
      
        FD_ZERO(&writefds);
        FD_SET(fdR0, &writefds);
        FD_SET(fdR1, &writefds);

        /* Wait up to five seconds. */
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        retval = select(maxfd + 1, &readfds, NULL, NULL, &tv); //here is select, second slot is for
                                                        //for write checks
 
        /* Don’t rely on the value of tv now! */
        if (retval == -1)
            perror("select()");
            else if (retval > 0) 
            {
                    printf("Data is available now.\n");
         

                    if (FD_ISSET(fdW0, &readfds)) 
                    {
                        ssize_t bytesB= read(fdW0, strW0, sizeof(strW0)-1); 
                        

                     if (bytesB <= 0) 
                        {
                            printf("Pipe closed\n");
                            break;
                        }

                     strW0[bytesB]='\0';
        
                     /* read numbers from input line */
                     sscanf(strW0, format_stringW0, s1 );
                     counts1++;
                    
                    if (counts1=0) 
                    {
                        old_s1[countw1] = s1[0];
                    }

                    } 

                    if (FD_ISSET(fdW1, &readfds)) 
                    {
                        ssize_t bytesB= read(fdW1, strW1, sizeof(strW1)-1); 
                        

                     if (bytesB <= 0) 
                        {
                            printf("Pipe closed\n");
                            break;
                        }

                     strW1[bytesB]='\0';
        
                     /* read numbers from input line */
                     sscanf(strW1, format_stringW1, s2 );
                     counts2++;

                     if (counts2=0) 
                    {
                        old_s2[countw2] = s2[0];
                    }
                     //strcpy(old_s2,s2);
                    }

                    //now we compare what was written so that either R0 or R1 takes it
                 
                if (counts1>=0 && counts2>=0)
                

                {
                    if (strcmp(s1, s2) == 0 )
                    {
                        countw1++;
                        countw2++;
                        old_s1[countw1] = s1[0]; // Add the new character to the end
                        old_s2[countw2] = s2[0]; // Add the new character to the end
                        // read(fdW0[0], buf, sizeof(buf));
                        // read(fdW0[0], buf, sizeof(buf));
                        printf("\nReceived from W0 %s\n", strW0);
                        
                        printf("\nReceived from W1 %s\n", strW1);
                        

                    }

                    else if (strcmp(s1, s2) > 0) //logical return, s1 has most recent value
                    {
                        countw1++;
                        old_s1[countw1] = s1[0];   
                        printf("\nReceived from W0 %s\n", strW0);
                       
                        if (s1[0] != old_s1[countw1-1])
                    {
                        if (FD_ISSET(fdR0, &writefds)) 
                        {
                            write(fdR0, s1, strlen(s1)+1);
                        }
                         
                    }
                    }        
                    else
                    {
                        countw2++;
                        old_s2[countw2] = s2[0];
                        printf("\nReceived from W1 %s\n", strW1);
                       
                        if (s2[0] != old_s2[countw2-1])
                    {
                        if (FD_ISSET(fdR1, &writefds)) 
                        {
                            write(fdR1, s2, strlen(s2)+1);
                        } 
                    
                    }
                    }    
                                
                    //lets compare
                 

                    
                }
            
            }

        else
            printf("No data within five seconds.\n");
        
    }
    
    
}

 
 