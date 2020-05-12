#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>

// globals
#define true (1==1)
#define false (1==0)

int Idx;
int ProgramCount;
pid_t pid[5];

void handler(int signal, siginfo_t *info, void *context) {
	printf("Handler Process: %i - Received Signal: %d\n", getpid(), signal);
	pid_t wait;
	int waitStatus;
	while (true) {
		if (Idx < 0) {
			Idx = ProgramCount - 1;
		}

		wait = waitpid(pid[Idx], &waitStatus, WNOHANG);
		if (waitStatus != -1) {
			printf("Sending stop signal to pid: %d\n", pid[Idx]);
			kill(pid[Idx], SIGSTOP);
			Idx--;
			break;
		}
		else
		{
			Idx--;
		}
	}

	while (true) {
		if (Idx < 0) {
			Idx = ProgramCount - 1;
		}

		wait = waitpid(pid[Idx], &waitStatus, WNOHANG);
		if (waitStatus != -1) {
			printf("Sending continue signal to pid: %d\n", pid[Idx]);
			kill(pid[Idx], SIGCONT);
			Idx--;
			break;
		}
		else
		{
			Idx--;
		}
	}
}

void signaler(pid_t *pid, int signal, int shift) {
  int i = 0;
  for (i; i < ProgramCount-shift; i++) {
    printf("Received Signal %d to pid %d\n", signal, pid[i]);
		kill(pid[i], signal);
  }
}

void print_status(pid_t pid) {
	char read[50] = "/proc/";
	FILE *in;


	snprintf(read, 50, "/proc/%d/status", pid);
	in = fopen(read, "r");

	// check for in errors
	if (in == NULL) {
		return;
	}

	int line_iterator = 0;
	char* line = NULL;
	size_t length = 0;
	while (line_iterator < 12) {
		getline(&line, &length, in);
		if (strcmp(line, "\n") == 0) {
			continue;
		}

		if (strcmp(line, "\0") == 0) {
			continue;
		}

		if (strcmp(line, "\0\n") == 0) {
			continue;
		}

		printf("%s", line);
		line_iterator++;
	}

	fclose(in);
	free(line);
}

int all_exited(pid_t *pid) {
	int arr[ProgramCount];
	pid_t wait;
	int waitStatus;
	int i;
	for ( i = ProgramCount; i > 0; i-- ){
		// status
		print_status(pid[i]);
		sleep(3);

		system("clear");
		wait = waitpid(pid[i], &waitStatus, WNOHANG);
		arr[i] = wait;
	}

	for (i = ProgramCount ; i > 0; i--) {
		if (arr[i] != -1) {
			return 1;
		}
	}
	return 0;
}

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

  // sigaction one declaration
  struct sigaction sig = {};
  memset(&sig, '\0', sizeof(&sig));
  sig.sa_sigaction = &handler;
  sig.sa_flags = SA_SIGINFO;
  sigaction(SIGALRM, &sig, NULL);

	// sigaction one declaration
	struct sigaction sigTwo = {};
	memset(&sigTwo, '\0', sizeof(&sigTwo));

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

  ProgramCount = pc;
	Idx = ProgramCount - 1;

  in = fopen(argv[1], "r");
  int j = 0;
  pid[pc];
  pid_t wait;
  int waitStatus;

  while ((number = getline(&line, &length, in)) != -1) {
    // setup signal
    int sigInt;
    int *sigPtr = &sigInt;
    int waitReturn;
    int completed;

    sigset_t set;
    sigemptyset(&set);

    completed = sigaddset(&set, 10);
    if (completed != 0) {
      printf("Error!: signal could not be added to the set\n");
    }

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

    // execute child
    if (pid[j] == 0) {
			sigaction(SIGUSR1, &sigTwo, NULL);

      waitReturn = sigwait(&set, sigPtr);
      if (waitReturn > 0) {
        printf("Error!: sigwait failed\n");
      }

      execvp(arguements[0], arguements);
      printf("\nError!: Invalid executable\n\n");
    }

    j++; // iterate pid iterator
  }
  sleep(1);
  signaler(pid, SIGUSR1, 0);
  sleep(1);
  signaler(pid, SIGSTOP, 1);
	time_t now, elapsed;
	int k;
	while (true) {
		int exited;

		now = time(NULL);
		elapsed = time(NULL) + 1;
		alarm(1);

		while (true) {
			now = time(NULL);
			if (now > elapsed) {
				elapsed = time(NULL) + 1;
				break;
			}
		}

		exited = all_exited(pid);

		if (exited) {
			break;
		}
	}

  // free all the things
  free(token);
  free(line);
  fclose(in);
  return 1;
}
