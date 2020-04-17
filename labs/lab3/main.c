/*
* Description: <write a brief description of your lab>
*
* Author: <your name>
*
* Date: <todacommand.h.gchy's date>
*
* Notes:
* 1. <add notes we should consider when grading>
*/

/*-------------------------Preprocessor Directives---------------------------*/
#define  _GNU_SOURCE
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "command.h"
/*---------------------------------------------------------------------------*/
#define true (1==1)
#define false (1==0)
#define debug false

char *left_trim(char *s){ while(isspace(*s)) s++; return s; }
char *right_trim(char *s){ char *back = s + strlen(s); while(isspace(*--back)); *(back+1) = '\0'; return s; }
char *trim(char *s){ return right_trim(left_trim(s));  }
/*---------------------------------------------------------------------------*/
void execute(char *line)
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
	for (size_t a = 0; a <= token_array_length; a++) {
		for (size_t b = 0; b <= token_array_length; b++) {
			final[a][b] = "";
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
				final[k][iterator] = tok; // update final and add token
				tok = strtok(NULL, " ");
				iterator++;
			}
		}
	}

	for (size_t a = 0; a <= token_array_length; a++) {
		for (size_t b = 0; b <= token_array_length; b++) {
			if (strcmp(final[a][b], "") != 0) // remove any empty records
			{
				if (b == 0) // ensure that we are dealing with the cmd
				{
					if (strcmp(final[a][b], "lfcat") == 0)
					{
						if (strcmp(final[a][1], "") != 0) {
							printf("Error! extra parameters\n");
							return;
						}
						lfcat();
					}
					else
					{
						printf("Error! Unrecognized command: %s\n", final[a][0]);
					}
				}
			}
		}
	}
}

void interative()
{
	char *input = NULL;
  // size of the inputs
  size_t len = 0;

  // clear the std out buffer
  setbuf(stdout, NULL);
  while (true)
  {
    printf(">>> "); // print the input notation
    getline(&input, &len, stdin); // get the input line and set size from standard in

    // special check for exit
    if (strcmp(input, "exit\n") == 0)
    {
      free(input); // unallocate the input
      exit(EXIT_SUCCESS); // exit the application
      return;
    }

    // execute commands
    execute(input); // execute that shit
  }
  free(input);
	return;
}

void file(char *file)
{
	FILE *o = freopen("output.txt", "a+", stdout);

	int exists = open(file, O_RDONLY);
	// file does not exist
	if (exists == -1) {
		printf("Error!: Unrecognized run command.\n");
		exit(EXIT_FAILURE);
	}

	// file does exist
  FILE *f = fopen(file, "r");
  // something went wrong above
  if(f == NULL)
  {
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

	return;
}
/*-----------------------------Program Main----------------------------------*/
int main(int argc, char *argv[]) {
	if(argc == 1)
  {
    // no flags thrown
    // while loop the data
    interative();
    return 1;
  }
  else if (argc >= 1)
  {
    // flags thrown
    if (strcmp(argv[1], "-f") == 0)
    {
			file(argv[2]);
      return 1;
    }
  }

	/*Free the allocated memory and close any open files*/
	return 0;
}
/*-----------------------------Program End-----------------------------------*/
