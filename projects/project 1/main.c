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

// globals
#define true (1==1)
#define false (1==0)
#define debug false
#define interactive_input stdin
#define interactive_output stdout
#define file_output "output.txt"
const char file_delimiter[2] = " ";

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
  if (debug) { printf("<<< COMPILING TOKENS\n"); }
  char *tokenize_initial = strtok(line, ";");

  while(tokenize_initial != NULL)
  {

    if (debug) { printf("<<< token(%s)\n", trim(tokenize_initial)); }
    // execute the commands
    if (strcmp(trim(tokenize_initial), "ls") == 0){
      listDir();
    }
    else if (strcmp(trim(tokenize_initial), "pwd") == 0)
    {
      showCurrentDir();
    }
    else
    {
      printf("Error! Unrecognized command\n");
    }

    tokenize_initial = strtok(NULL, ";");
  }

  if (debug) { printf("<<< FINISHING WITH TOKENS\n"); }

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
