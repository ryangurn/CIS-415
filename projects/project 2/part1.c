#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>

// globals
#define true (1==1)
#define false (1==0)

int main(int argc, char const *argv[])
{
  // clear stdout
  setbuf(stdout, NULL);

  char *line = NULL;
  size_t number;
  size_t length = 0;
  char *token;
  char *arguements[10];
  int i = 0;

  FILE *in;

  // check input parameters
  if (argc == 1 || argc < 2) {
    printf("Error: wrong command line arguments\n");
    return 1;
  }

  // open the input file
  in = fopen(argv[1], "r");
  // check for input file errors
  if (in == NULL) {
    printf("Error: cannot open input file\n");
    return 1;
  }

  int pc = 0;
  // determine the number of lines (ie a program counter)
  while ((number = getline(&line, &length, in)) != -1) {
    pc++; // add to the program counter
  }

  fclose(in); // close the input just for a moment

  in = fopen(argv[1], "r");
  int j = 0;
  pid_t pid[pc]; // setup the correct numbers of pid's
  pid_t wait;
  int waitStatus;

  while ((number = getline(&line, &length, in)) != -1) {
    i = 0; // set position back to zero for use again
    for (; i < 10; i++) {
      arguements[i] = NULL; // setup empty array with NULL ptrs
    }

    i = 0; // set position back to zero for use again
    token = strtok(line, " "); // start parsing tokens
    while (token != NULL) {
      // remove new line from the end of the token
      if (token[strlen(token)-1] == '\n') {
        token[strlen(token)-1] = 0; // swap with a zero
      }

      arguements[i] = token; // assign to position in arr
      token = strtok(NULL, " "); // move token forward
      i++; // interate position in arr forward
    }

    pid[j] = fork(); // fork

    if (pid[j] < 0) { // check for fork errors
      exit(-1); // exit if needed
    }

    // execute child
    if (pid[j] == 0) {
      execvp(arguements[0], arguements);
      printf("Error!: Invalid Executable\n");
      exit(-1);
    }

    j++; // iterate pid iterator
  }

  // wait
  int waitIterator = 0;
  for (waitIterator; waitIterator < pc; waitIterator++) {
    wait = waitpid(pid[waitIterator], &waitStatus, WUNTRACED | WCONTINUED);
  }

  // free all the things
  free(token);
  free(line);
  fclose(in);
  return 1;
}
