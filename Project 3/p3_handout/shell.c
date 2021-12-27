/* This is the only file you should update and submit. */

/* Fill in your Name and GNumber in the following two comment fields
 * Name: Siddharthkumar Patel
 * GNumber: G01075115
 */

#include "shell.h"
#include "parse.h"

void help_shell(char *cmdline);
void quit_shell(char *cmdline);

/* Constants */
#define DEBUG 0
/*
 * static const char *shell_path[] = { "./", "/usr/bin/", NULL };
 * static const char *built_ins[] = { "quit", "help", "kill", 
 * "fg", "bg", "jobs", NULL};
*/

/**
 * This is a job struct or a linkedlist taht stores a list of foreground
 * or background jobs.
 */
struct job_g {
	pid_t pid;
	int jobid;
	int state;
	char cmd[MAXLINE];
};

/* The entry of your shell program */
int main() {
  char cmdline[MAXLINE];        /* Command line */
  char *cmd = NULL;

  /* Initial Prompt and Welcome */
  log_prompt();
  log_help();


  /* Shell looping here to accept user command and execute */
  while (1) {
    char *argv[MAXARGS], *argv2[MAXARGS];     /* Argument list */
    Cmd_aux aux;                /* Auxilliary cmd info: check parse.h */

    /* Print prompt */
    log_prompt();

    /* Read a line */
    // note: fgets will keep the ending '\n'
    if (fgets(cmdline, MAXLINE, stdin) == NULL) {
      if (errno == EINTR)
        continue;
      exit(-1);
    }

    if (feof(stdin)) {  /* ctrl-d will exit shell */
      exit(0);
    }

    /* Parse command line */
    if (strlen(cmdline)==1)   /* empty cmd line will be ignored */
      continue;     

    cmdline[strlen(cmdline) - 1] = '\0';        /* remove trailing '\n' */

    cmd = malloc(strlen(cmdline) + 1);
    snprintf(cmd, strlen(cmdline) + 1, "%s", cmdline);

    /* Bail if command is only whitespace */
    if(!is_whitespace(cmd)) {
      initialize_argv(argv);    /* initialize arg lists and aux */
      initialize_argv(argv2);
      initialize_aux(&aux);
      parse(cmd, argv, argv2, &aux); /* call provided parse() */

      if (DEBUG)  /* display parse result, redefine DEBUG to turn it off */
        debug_print_parse(cmd, argv, argv2, &aux, "main (after parse)");

      /* After parsing: your code to continue from here */
      /*================================================*/

			help_shell(cmdline);
			quit_shell(cmdline);
    }

    free_options(&cmd, argv, argv2, &aux);
  }
  return 0;
}

void help_shell(char *cmdline) {
	if(strcmp(cmdline, "help") == 0) {
		log_help();
	}
}

void quit_shell(char *cmdline) {
	if(strcmp(cmdline, "quit") == 0) {
		log_quit();
		exit(0);
	}
}
