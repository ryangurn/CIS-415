#define  _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "command.c"
#include <sys/syscall.h>

// these two imports are for the open function and the close function
// reference: * https://pubs.opengroup.org/onlinepubs/007904975/functions/open.html
//            * https://pubs.opengroup.org/onlinepubs/007904975/functions/close.html
#include <fcntl.h> // for open
#include <unistd.h> // for close

// globals
#define true (1==1)
#define false (1==0)
#define debug true
#define file_input stdin
#define file_output stdout

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
  setbuf(file_output, NULL);
  while (true)
  {
    printf(">>> "); // print the input notation
    getline(&input, &len, file_input); // get the input line and set size from standard in

    if (debug) { printf("<<<(%ld) %s\n", len, input); } // debug

    // special check for exit
    if (strcmp(input, "exit\n") == 0)
    {
      if (debug) { printf("<<< EXIT_SUCCESS\n"); } // debug

      free(input); // unallocate the input
      exit(EXIT_SUCCESS); // exit the application
      return 1;
    }

  }
  return 0;
}

int file(char *file)
{
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
