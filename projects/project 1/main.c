// stupid error: warning: implicit declaration of function ‘getline’; did you mean ‘getlogin’?
#define  _GNU_SOURCE
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>

// these two imports are for the open function and the close function
// reference: * https://pubs.opengroup.org/onlinepubs/007904975/functions/open.html
//            * https://pubs.opengroup.org/onlinepubs/007904975/functions/close.html
#include <fcntl.h> // for open
#include <unistd.h> // for close

// includes for commands
#include "command.h"

// globals
#define true (1==1)
#define false (1==0)
#define debug true
#define interactive_input stdin
#define interactive_output stdout
#define file_output "output.txt"

FILE *output = stdout;

// helper functions
char *left_trim(char *s){ while(isspace(*s)) s++; return s; }
char *right_trim(char *s){ char *back = s + strlen(s); while(isspace(*--back)); *(back+1) = '\0'; return s; }
char *trim(char *s){ return right_trim(left_trim(s));  }

// execute()
// Purpose: the purpose of this function is to ensure that execution of the commands are done consistently
//          since there is overlap between file and interactive mode, this will ensure the code stays DRY
// DRY Reference: https://en.wikipedia.org/wiki/Don%27t_repeat_yourself
// Design Rationale: the rationale for this function is mainly to ensure that code is staying DRY and that
//                   will also have payoff's for debugging/performance/etc.
int execute(char *line)
{
  // declare some counters
  int i = 0;
  int count = 0;

  // determine the number of tokens needed.
  size_t token_array_length = 0;
  char *ptr = line;
  while((ptr = strchr(ptr, ' ')) != NULL) {
      token_array_length++;
      ptr++;
  }

  // declate the arrays that we will use to tokenize.
  char *array[sizeof(line)];
  char *final[sizeof(line)][sizeof(line)];
  // define the final 2d array that information gets placed within.
  if(debug){
    for (size_t a = 0; a <= token_array_length; a++) {
      for (size_t b = 0; b <= token_array_length; b++) {
        final[a][b] = "";
      }
    }
  }

  char *token = strtok(line, ";");
  while (token != NULL)
  {
    array[i] = trim(token);
    i++;
    count++;
    token = strtok(NULL, ";");
  }

  for (int k = 0; k < count; k++) {
    if (strcmp(array[k], ""))
    {
      char *tok = strtok(array[k], " ");
      int iterator = 0;
      while (tok != NULL)
      {
        if (debug) { printf("<<< (%d)(%d) %s\n", k, iterator, tok); }

        final[k][iterator] = tok; // update final and add token

        tok = strtok(NULL, " ");
        iterator++;
      }
    }
  }

  // print out the contents of final
  for (size_t a = 0; a <= token_array_length; a++) {
    for (size_t b = 0; b <= token_array_length; b++) {
      printf("[%ld][%ld] - %s ", a, b, final[a][b]);
    }
    printf("\n");
  }

  for (size_t a = 0; a <= token_array_length; a++) {
    for (size_t b = 0; b <= token_array_length; b++) {
      if (strcmp(final[a][b], "") != 0) // remove any empty records
      {
        if (b == 0) // ensure that we are dealing with the cmd
        {
          if (debug) {
            printf("<< %s\n", final[a][b]);
          } // debug
          if (strcmp(final[a][b], "ls") == 0)
          {
            // TODO: ADD EMPTY PARAM CHECK
            // if (strcmp(final[a][1], "") != 0) { printf("Error! Unsupported parameters for command: %s\n", final[a][0]); return 0; }
            listDir();
          }
          else if (strcmp(final[a][b], "pwd") == 0)
          {
            // TODO: ADD EMPTY PARAM CHECK
            // if (strcmp(final[a][1], "") != 0) { printf("Error! Unsupported parameters for command: %s\n", final[a][0]); return 0; }
            showCurrentDir();
          }
          else if (strcmp(final[a][b], "mkdir") == 0)
          {
            if (strcmp(final[a][1], "") != 0)
            {
              if(debug) { printf("<<< %s\n", final[a][1]); }
              makeDir(final[a][1]);
            } else {
              printf("Error! Missing parameter for command: %s\n", final[a][0]); return 0;
            }
          }
          else if (strcmp(final[a][b], "cd") == 0)
          {
            if (strcmp(final[a][1], "") != 0)
            {
              if(debug) { printf("<<< %s\n", final[a][1]); }
              changeDir(final[a][1]);
            } else {
              printf("Error! Missing parameter for command: %s\n", final[a][0]); return 0;
            }
          }
          else if (strcmp(final[a][b], "cp") == 0)
          {
              if (strcmp(final[a][1], "") != 0 && strcmp(final[a][2], "") != 0)
              {
                if (debug) { printf("<<< %s - %s\n", final[a][1], final[a][2]); }
                copyFile(final[a][1], final[a][2]);
              } else {
                printf("Error! Missing parameters for command: %s\n", final[a][0]); return 0;
              }
          }
          else if (strcmp(final[a][b], "mv") == 0)
          {
            if (strcmp(final[a][1], "") != 0 && strcmp(final[a][2], "") != 0)
            {
              if (debug) { printf("<<< %s - %s\n", final[a][1], final[a][2]); }
              moveFile(final[a][1], final[a][2]);
            } else {
              printf("Error! Missing parameters for command: %s\n", final[a][0]); return 0;
            }
          }
          else if (strcmp(final[a][b], "rm") == 0)
          {
            if (strcmp(final[a][1], "") != 0)
            {
              if (debug) { printf("<<< %s\n", final[a][1]); }
              deleteFile(final[a][1]);
            } else {
              printf("Error! Missing parameters for command: %s\n", final[a][0]); return 0;
            }
          }
          else if (strcmp(final[a][b], "cat") == 0)
          {
            if (strcmp(final[a][1], "") != 0)
            {
              if (debug) { printf("<<< %s\n", final[a][1]); }
              displayFile(final[a][1]);
            } else {
              printf("Error! Missing parameters for command: %s\n", final[a][0]); return 0;
            }
          }
          else
          {
            printf("Error! Unrecognized command: %s\n", final[a][0]);
          }

        }
      }
    }
  }

  return 0;
}

// interactive()
// Purpose: this function will run the interactive mode for the application
// Design Rationale: ensure that all functionality for interactive mode is tightly coupled
int interative()
{
  // main vars
  // input is meant to be the var that all inputs are put into
  char *input = NULL;
  // size of the inputs
  size_t len = 0;

  // clear the std out buffer
  setbuf(interactive_output, NULL);
  while (true)
  {
    printf(">>> "); // print the input notation
    getline(&input, &len, interactive_input); // get the input line and set size from standard in

    if (debug) { printf("<<<(%ld) %s\n", len, input); } // debug

    // special check for exit
    if (strcmp(input, "exit\n") == 0)
    {
      if (debug) { printf("<<< EXIT_SUCCESS\n"); } // debug

      free(input); // unallocate the input
      exit(EXIT_SUCCESS); // exit the application
      return 1;
    }

    // execute commands
    execute(input); // execute that shit
  }
  free(input);
  return 0;
}

// file()
// Args: * file - a string reference to the file that can be used and bassed.
// Purpose: this function will run the file mode for the application
// Design Rationale: ensure that all functionality for file mode is tightly coupled
int file(char *file)
{
  // open the output file
  // reference: http://www.decompile.com/cpp/faq/fopen_write_append.htm
  FILE *o = fopen(file_output, "a+"); // append new information or create file if it does not exists

  // attempt to open the file based on str
  int exists = open(file, O_RDONLY);
  if (debug) { printf("<<< EXISTS: (%d)\n", exists); }

  // file does not exist
  if (exists == -1) {
    if (debug) { printf("<<< FILE DOES NOT EXIST, EXITING\n"); }
    printf("Error!: Unrecognized run command.\n");
    exit(EXIT_FAILURE);
  }

  // file does exist
  FILE *f = fopen(file, "r");
  // something went wrong above
  if(f == NULL)
  {
    if (debug) { printf("<<< WE SHOULD NOT GET HERE... RUN GDB\n"); }
    printf("Error!: Unknown error occurred.\n");
  }

  // finally we can get the contents!
  char *line = NULL;
  size_t line_length = 0;
  getline(&line, &line_length, f); // get the line the first time
  do{
    execute(line); // execute that shit
  } while(getline(&line, &line_length, f) != -1); // iterate until we end

  free(line);

  // close the files
  fclose(f); // close the input file
  fclose(o);


  return 0;
}

// main()
// Args: * argc - arguement count as provided by standard library
//       * argv - string arguements that have been passed
// Purpose: bootstrap the application
// Design Rationale: this function is completely required by c standard library
int main(int argc, char *argv[])
{
  if(argc == 1)
  {
    if (debug) { printf("<<< INTERACTIVE\n"); } // debug
    // no flags thrown
    // while loop the data
    interative();
    return 1;
  }
  else if (argc >= 1)
  {
    if (debug) {
      printf("<<< NON INTERACTIVE\n");
      printf("<<< argv[1] (%s)\n", argv[1]);
    } // debug
    // flags thrown
    if (strcmp(argv[1], "-f") == 0)
    {
      if (debug) {
        printf("<<< FILE MODE\n");
        printf("<<< argv[2] (%s)\n", argv[2]);
      } // debug
      return file(argv[2]);
    }
  }
}
