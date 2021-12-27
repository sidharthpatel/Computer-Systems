#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <sys/types.h>
//#include <sys/wait.h>

void sigint_handler(int sig) { /* signal handler for SIGINT */
  write(STDOUT_FILENO, "SIGINT\n", 7);
  exit(0);
}

int main(void) {
  struct sigaction new; /* sigaction structures */
  struct sigaction old;

  new.sa_handler = sigint_handler; /* set the handler */
  new.sa_flags = 0;

  sigaction(SIGINT, &new, &old); /* register the handler for SIGINT */

  int i = 0;
  while (i < 100000) {          /* this will loop for a while */
    fprintf(stderr, "%d\n", i); /* break loop by Ctrl-c to trigger SIGINT */
    sleep(1);
    i++;
  }

  return 0;
}
