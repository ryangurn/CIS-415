#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    setbuf(stdout, NULL); // clear the output buffer.
    
    char *line = NULL;
    size_t len = 0;
    
    if (argc != 2) {
        printf("Error!: Wrong command line arguments\n");
        return 1;
    }
    
    // open the input file using the argv parameter
    FILE *input = fopen(argv[1], "r");
    if (input == NULL)
    {
        printf("Error!: Unable to open input file");
        return 1;
    }
    
    // count the number of lines from the input
    int count = 0;
    while((l = getline(&line, &len, input) != -1)
    {
        count++;
    }
    
    // setup an arguments arr
    int args[5];
    size_t args_len = 5;
    for (int i = 0; i < args_len; i++)
    {
        args[i] = NULL; // init value
    }
    
    int fork_id = 0;
    pid_t pid[count];
    while((l = getline(&line, &len, input) != -1)
    {
        int ind = 0;
        char *token = strtok(line, " ");
        while (token != NULL)
        {
            // replace new lines with a zero
            if (token[strlen(token)-1] == '\n')
            {
                token[strlen(token)-1] = 0; // replacement
            }
            
            args[ind] = token; // put the token into the args arr
            token = strtok(NULL, " "); // get the next token
            ind++; // keep moving forward
        }
        
        // time to start forking and check for errors
        pid[fork_id] = fork();
        if (pid[fork_id] < 0) 
        {
            exit(-1); // exit the process
        }
        
        // check if the current process is a child
        if (pid[fork_id] == 0)
        {
            execvp(args[0], args);
            printf("\nError: Invalid Executable\n");
            exit(-1);
        }
        
        
        fork_id++;
    }
    
    pid_t wait;
    int status;
    for (int i = 0; i < count; i++)
    {
        status = waitpid(pid[i], &wait, WUNTRACED | WCONTINUED);
    }
    
    // close the input file.
    fclose(input);
    
    // free some stuff
    free(line);
    // TODO: ensure that we do not need to free anything else with valgrind once the VM is accessible.
    
    return 0;
}