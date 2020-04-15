/*
* Description: <write a brief description of your lab>
*
* Author: <your name>
*
* Date: <today's date>
*
* Notes:
* 1. <add notes we should consider when grading>
*/

/*-------------------------Preprocessor Directives---------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include "command.h"
/*---------------------------------------------------------------------------*/

/*-----------------------------Program Main----------------------------------*/
int main() {
	setbuf(stdout, NULL);

	// Variable declarations
	condition = 1;

	/* Switch the context of STDOUT if the program is started with the -f flag.
	   Open the file for reading and open a file "output.txt" for writing.
	   Hint: use freopen() to switch the context of a file descriptor. */

		 // Remove the newline at the end of the input string.
	 	//Hint: use strcspn(2) to find where the newline is then assign '\0' there.

	// Main run cycle
	do {


		// Display prompt and read input from console/file using getline(3)

		/* Tokenize and process the input string. Remember there can be multiple
		   calls to lfcat. i.e. lfcat ; lfcat <-- valid
		 	 If the command is either 'exit' or 'lfcat' then do the approp. things.
		   Note: you do not need to explicitly use an if-else clause here. For
			     instance, you could use a string-int map and a switch statement
					 or use a function that compares the input token to an array of
					 commands, etc. But anything that gets the job done gets full points so
					 think creatively. ^.^  Don't forget to display the error messages
					 seen in the lab discription*/

	} while(condition);

	/*Free the allocated memory and close any open files*/
	return 0;
}
/*-----------------------------Program End-----------------------------------*/
