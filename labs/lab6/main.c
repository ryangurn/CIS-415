#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <wait.h>

// globals
#define true (1==1)
#define false (1==0)

void handler(int signal)
{
  printf("Child Process: %i - Waiting for SIGUSR1...\n", getpid());

  // store the signal
  int sigInt;
  int *sigPtr = &sigInt;

  int waitRet;
  int completed;

  sigset_t set;
  sigemptyset(&set);

  completed = sigaddset(&set, signal);
  if ( completed != 0 )
  {
    printf("Error: Signal unable to be added\n");
  }

  if ( signal != SIGUSR1 )
  {
    exit(-1);
  }
  else
  {
    printf("Child Process: %i - Received signal: SIGUSR1 - Calling exec().\n", getpid());
    // sigwait()
    char *params[1] = {"iobound"};
    execvp("iobound", params);
    waitRet = sigwait(&set, sigPtr);
    if(waitRet > 0)
    {
      printf("Error: sigwait() failed\n");
    }
  }
}

void signaler(pid_t *pid, int signal)
{
  size_t i = 0;
  for (; i < 5; i++) {
    sleep(1);
    printf("Parent process: %i - Sending signal: %d to child process: %i\n", getpid(), signal, pid[i]);
    kill(pid[i], signal);
  }
}

int main(int argc, char const *argv[]) {
  pid_t pid[5];
  pid_t w;
  int waitStatus;

  // https://github.com/nsf/termbox/issues/35
  struct sigaction sig = {};
  memset(&sig, '\0', sizeof(&sig));
  sig.sa_handler  = &handler;
  sig.sa_flags = SA_SIGINFO;

  size_t i = 0;
  // setup pool
  for (; i < 5; i++)
  {

    // fork and add to pid_t arr
    pid[i] = fork();

    if(pid[i] < 0) {

      perror("Error! could not create a new process.\n");
      exit(EXIT_FAILURE);
    }
    else if(pid[i] == 0)
    {
      // Child
      sigaction(SIGUSR1, &sig, NULL);
      while(true)
      {
        sleep(1);
      } // end while

    }
    else if(pid[i] > 0)
    {
      printf("Parent process: %d - Sending signals to child...\n", getpid());
      int j = 0;
      printf("Parent process: %d - Waiting for child to complete...\n", getpid());
      for(j; j < 11; j++)
      {
        signaler(pid, SIGSTOP);
        sleep(3);
        signaler(pid, SIGCONT);
        sleep(1);
      }
      w = waitpid(pid[i], &waitStatus, 0);
      printf("Parent process: %d - Finished\n", getpid());
    }

  } // end for

  // start things off
  for (i = 0; i < 5; i++) {
    printf("Parent Process: %i - Killing Child: %i\n", getpid(), pid[i]);
    w = waitpid(pid[i], &waitStatus, WUNTRACED | WCONTINUED);
    printf("Exiting\n");
  }
} // end func
