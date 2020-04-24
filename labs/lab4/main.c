#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
  // when changing this to have multiple processes we want an arr of pid_t
  pid_t pid;

  printf("Main start: PID(%d)\n", getpid());

  pid = fork();

  if ( pid == 0 )
  {
    printf("Child Process: PID(%d) | Parent Process: PID(%d)\n", getpid(), getppid());
    printf("Status: %d\n", pid);

    // execute another binary using the following
    // if (execv("./helloWorld") < 0)
    // {
    //   perror("execv")
    // }

    sleep(2);
  }
  else
  {
    // ensure that only the parent forks and that happens in this case
    wait(0);
    printf("Main exiting: PID(%d)\n", getpid());
  }

  return 0;
}
