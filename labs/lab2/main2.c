/*
* Description: This is lab 2, its supposed to help with the project 1, but im
* already finished with project 1 so i guess im here doing this after project 1
* has been completed.
*
* Author: Ryan Gurnick
*
* Date: 04/8/2020
*
* Notes:
* 1. Nothing really, this was pretty straight forward.
*/

/*-------------------------Preprocessor Directives---------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/*---------------------------------------------------------------------------*/

// helper functions
char *left_trim(char *s){ while(isspace(*s)) s++; return s; }
char *right_trim(char *s){ char *back = s + strlen(s); while(isspace(*--back)); *(back+1) = '\0'; return s; }
char *trim(char *s){ return right_trim(left_trim(s));  }

/*-----------------------------Program Main----------------------------------*/
int main(int argc, char *argv[]) {
	// error checking for argv and argc
	if (argc > 2 || argv[1] == NULL)
	{
		printf("Too many arguements provided\n");
		exit(EXIT_FAILURE);
	}

	/* Main Function Variables */
	char *input = NULL;
	size_t len = 0;
	/* Allocate memory for the input buffer. */
	setbuf(stdout, NULL);
	/*main run loop*/
	while (1)
	{
		/* Print >>> then get the input string */
		printf(">>> ");
		/* Get the input */
		getline(&input, &len, stdin);

		if (strcmp(input, "exit\n") == 0)
    {
      free(input); // unallocate the input
      exit(EXIT_SUCCESS); // exit the application
      return 1;
    }

		/* Tokenize the input string */
		char *token = strtok(input, " ");
		/* Display each token */
		int count = 0;

		FILE *output;
		if (strcmp(argv[1], "") != 0)
		{
			output = fopen(argv[1], "w");
			if (output == NULL)
			{
				printf("Unable to open the file\n");
				exit(EXIT_FAILURE);
			}
		}

		while (token != NULL)
		{
			if (strcmp(argv[1], "") != 0 && output != NULL)
			{
					fprintf(output, "T%d: %s\n", count, trim(token));
			} else {
					printf("T%d: %s\n", count, trim(token));
			}
			token = strtok(NULL, " ");
			count++;
		}
		if (output != NULL)
		{
			fclose(output);
		}
		/* If the user entered <exit> then exit the loop */
	}

	/*Free the allocated memory*/
	free(input);
	exit(EXIT_SUCCESS);

	return 1;
}
/*-----------------------------Program End-----------------------------------*/
