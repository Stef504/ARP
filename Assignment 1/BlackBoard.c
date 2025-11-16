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

int rph_intial;
double eta_intial;
int force_intial;
int mass;
int k_intial;
int working_area;
int t_intial;

void Parameter_File() {
    FILE* file = fopen("Parameter_File.txt", "r");
    if (file == NULL) {
        perror("Error opening Parameter_File.txt");
        return;
    }

    char line[256];
    int line_number = 0;

    // --- 2. Read the file line by line ---
    while (fgets(line, sizeof(line), file)) {
        line_number++;

        // --- 3. Tokenize the *current* line ---
        char* tokens[10]; // An array to hold tokens for this ONE line
        int token_count = 0;
        char* token = strtok(line, "_");

        while (token != NULL && token_count < 10) {
            tokens[token_count] = token; // Add token to our array
            token_count++;
            token = strtok(NULL, "_"); // Get next token
        }

        // --- 4. Assign values based on line number ---
        // We use a 'switch' to make it cleaner than many 'if' statements.
        // We also check 'token_count' to avoid crashing if a line is blank.
        switch (line_number) {
            case 3:
                if (token_count > 2) rph_intial = atoi(tokens[2]);
                break;
            case 4:
                if (token_count > 2) eta_intial = atof(tokens[2]); // Use atof() for doubles
                break;
            case 5:
                if (token_count > 2) force_intial = atoi(tokens[2]);
                break;
            case 6:
                if (token_count > 1) mass = atoi(tokens[1]); // You used index [1] here
                break;
            case 7:
                if (token_count > 2) k_intial = atoi(tokens[2]);
                break;
            case 8:
                if (token_count > 2) working_area = atoi(tokens[2]);
                break;
            case 9:
                if (token_count > 2) t_intial = atoi(tokens[2]);
                break;
        }
    }
    fclose(file);
}
  
int main(int argc, char *argv[]) {

    Parameter_File();
    if (argc < 3) 
    {
        fprintf(stderr, "Usage: %s <fd>\n", argv[0]);
        exit(1);
    }

    // Convert the argument to an integer file descriptor
    //order from main fdW0_str,fdW1_str, fdR0_str,fdR1_str
    int fdIn = atoi(argv[1]);
    char bufferIn[100];

    int fdOb = atoi(argv[2]);
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

    system("./Ref_Win &"); 

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
    
    


 
 