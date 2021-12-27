/* This is the only file you should update and submit. */

/* Fill in your Name and GNumber in the following two comment fields
 * Name: Siddharthkumar Patel
 * GNumber: G01075115
 */

#include "shell.h"
#include "parse.h"

// Macros to track Process Status
#define RUNNING 0
#define FOREGROUND 1
#define BACKGROUND 2
#define STOPPED 3
#define TERMINATED 4

/*
 * This struct stores the list of jobs which can later be returned upon the call of built-in
 * job(s) command.
 */
typedef struct job_g {
	int pid;
	int job_id;
	int state;
	int type;
	char cmdline[MAXLINE];
	struct job_g *next;
} Node;

/* Preprocessors */
void help_shell(char *cmdline);
void quit_shell(char *cmdline);
void kill_handler();
void sleep_handler();
void cont_handler();
void child_handler();
void kill_process(int sig);
void sleep_process(int sig);
void cont_process(int sig);
void child_process(int sig);
void rd_cmd(char *command, char *argv[], char *argv2[], Cmd_aux *aux);
void process_jobs(char *command, char *argv[], char *argv2[], Cmd_aux *aux);
void del_pid(int pid);

/* Constants */
#define DEBUG 0
static const char *shell_path[] = { "./", "/usr/bin/", NULL };
static const char *built_ins[] = { "quit", "help", "kill", "fg", "bg", "jobs", NULL};

// Tracks the number of background jobs.
int num_jobs = 0;

// Uses a linkedlist to store all the foreground and background jobs.
Node *jobs = NULL;

/**
 * Print method for testing purposes (ignore it, if you must).
 */
void print_list() {
	Node *temp = jobs;
	while(temp != NULL) {
		printf("PID: %d\n", temp -> pid);
		printf("Job ID: %d\n", temp -> job_id);
		printf("State: %d\n", temp -> state);
		printf("Type: %d\n", temp -> type);
		printf("Command Line: %s\n\n", temp -> cmdline);
		temp = temp -> next;
	}
}

// Registering Signal Handlers
void ctrlc_handler() {
	struct sigaction ctrlc;
	memset(&ctrlc, 0, sizeof(struct sigaction));
	ctrlc.sa_handler = kill_process; // Process that shall be called to override SIGINT signal.
	sigaction(SIGINT, &ctrlc, NULL);
}

void ctrlz_handler() {
	struct sigaction ctrlz;
	memset(&ctrlz, 0, sizeof(struct sigaction));
	ctrlz.sa_handler = sleep_process;
	sigaction(SIGTSTP, &ctrlz, NULL);
}

void cont_handler() {
	struct sigaction cont;
	memset(&cont, 0, sizeof(struct sigaction));
	cont.sa_handler = cont_process;
	sigaction(SIGCONT, &cont, NULL);
}


void child_handler() {
	struct sigaction child;
	memset(&child, 0, sizeof(struct sigaction));
	child.sa_handler = child_process;
	sigaction(SIGCHLD, &child, NULL);
}


/* The entry of your shell program */
int main() {
  char cmdline[MAXLINE];        /* Command line */
  char *cmd = NULL;

  /* Initial Prompt and Welcome */
  log_prompt();
  log_help();

	// Ctrl Z handler
	ctrlz_handler();
	// Ctrl C handler
	ctrlc_handler();
	// Child handler
	child_handler();

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

			/**
			 * Redirecting the main code to ensure clean coding.
			 */
			rd_cmd(cmd, argv, argv2, &aux);

    }

    free_options(&cmd, argv, argv2, &aux);
  }
  return 0;
}

/**
 * This function prints the number of jobs along with the linked list.
 */
void print_jobs() {
	Node *count_pt = jobs;
	int count = 0;
	while(count_pt != NULL) {
		count++;
		count_pt = count_pt -> next;
	}

	log_job_number(count);
	Node *temp = jobs;
	if(temp != NULL && temp -> job_id == 0) {
		temp = temp -> next;
	}

	while(temp != NULL) {
		if(temp -> state == 0) {
			log_job_details(temp -> job_id, temp -> pid, "Running", temp -> cmdline);
		}
		else if(temp -> state == 3) {
			log_job_details(temp -> job_id, temp -> pid, "Stopped", temp -> cmdline);
		}
		temp = temp -> next;
	}
}

/**
 * Runs the entire list of built-ins/ non-built-in commands.
 */
void rd_cmd(char *command, char *argv[], char *argv2[], Cmd_aux *aux) {
	sigset_t cover, prev;

	// Sigemptyset initializes the the signal set to be empty.
	sigemptyset(&cover);
	sigemptyset(&prev);


	// Sigaddset adds a signal to be blocked.
	sigaddset(&cover, SIGINT);
	sigaddset(&cover, SIGCHLD);
	sigaddset(&cover, SIGTSTP);

	// Using the SIG_BLOCK, signal sets added above will be blocked.
	sigprocmask(SIG_BLOCK, &cover, NULL);


	if(strcmp(built_ins[0], argv[0]) == 0) {
		// If the user enters the built-in help command, the help shall be provided.
		quit_shell(command);
	}
	else if(strcmp(built_ins[1], argv[0]) == 0) {
		// If the user desires to quit the program, we shall quit it.
		help_shell(command);
	}
	// Kill Process
	else if(strcmp(built_ins[2], argv[0]) == 0) {
		int pid = atoi(argv[2]);
		int sig = atoi(argv[1]);
		if(sig == 2) {
			kill(pid, SIGINT);
		}
		else if(sig == 9) {
			kill(pid, SIGKILL);
		}
		else if(sig == 18) {
			kill(pid, SIGCONT);
		}
		else if(sig == 20) {
			kill(pid, SIGTSTP);
		}
		log_kill(atoi(argv[1]), atoi(argv[2]));
	}
	// fg built-in command handling
	else if(strcmp(built_ins[3], argv[0]) == 0) {
		// Search for possible pid in background jobs.
		int job_id = atoi(argv[1]);
		Node *temp = jobs;
		Node *current = NULL;
		while(temp != NULL) {
			if(temp -> job_id == job_id) {
				current = temp;
				break;
			}
			temp = temp -> next;
		}
		if(current == NULL) {
			log_jobid_error(job_id);
			return;
		}
		log_job_fg(current -> pid, current -> cmdline);
		current -> job_id = 0;
		current -> type = FOREGROUND;
		if(current -> state == STOPPED) {
			current -> state = RUNNING;
			kill(current -> pid, SIGCONT);
		}
		else {
			current -> state = RUNNING;
			waitpid(current -> pid, &current -> state, 0);
			del_pid(current -> pid);
		}
		print_list();
	}
	// bg built-in command handling.
	else if(strcmp(built_ins[4], argv[0]) == 0) {
		int job_id = atoi(argv[1]);
		Node *temp = jobs;
		Node *current = NULL;
		print_list();
		while(temp != NULL) {
			if(temp -> job_id == job_id) {
				current = temp;
				break;
			}
			temp = temp -> next;
		}
		if(current == NULL) {
			log_jobid_error(job_id);
			return;
		}
		kill(temp -> pid, SIGCONT);
	}
	else if(strcmp(built_ins[5], argv[0]) == 0) {
		// Displays the list of background jobs
		print_jobs();
	}
	else {
		/* Allows the input of the program. */
		sigprocmask(SIG_UNBLOCK, &cover, NULL);
		process_jobs(command, argv, argv2, aux);
	}
	sigprocmask(SIG_UNBLOCK, &cover, NULL);
}

/**
 *	The Help function is a built-in command that prints the help log upon request
 *	in the shell.
 *	Parameter: consists of a string of input passed in the MASH to identify the
 *	desired command that needs to be executed.
 */
void help_shell(char *cmdline) {
	if(strcmp(cmdline, "help") == 0) {
		log_help();
	}
}

/**
 * The quit function is a built-in command that exits from the MASH shell.
 */
void quit_shell(char *cmdline) {
	if(strcmp(cmdline, "quit") == 0) {
		log_quit();
		exit(0);
	}
}

/**
 * Override Ctrl C through this method.
 */
void kill_process(int sig) {
	printf("Entered CTRL C signal handler.\n");
	sigset_t cover, prev;
	sigemptyset(&cover);
	sigemptyset(&prev);

	/**
	 * Adding necessary signals to prevent them from causing interputions
	 * while interacting during the process of another signal.
	 */
	sigaddset(&cover, SIGCHLD);
	sigaddset(&cover, SIGINT);
	sigaddset(&cover, SIGTSTP);
	sigprocmask(SIG_BLOCK, &cover, NULL);
	
	Node *temp = jobs;
	if(temp != NULL && temp -> job_id == 0) {
		log_ctrl_c();
		temp -> state = TERMINATED;
		kill(temp -> pid, SIGINT);
	}
	/* Unblocking the signals before exiting */
	sigprocmask(SIG_SETMASK, &prev, NULL);
}

/**
 * Override Ctrl Z through this method.
 */
void sleep_process(int sig) {
	printf("Entered CTRL Z signal handler.\n");
	sigset_t cover, prev;
	sigemptyset(&cover);
	sigemptyset(&prev);

	sigaddset(&cover, SIGCHLD);
	sigaddset(&cover, SIGINT);
	sigaddset(&cover, SIGTSTP);
	sigprocmask(SIG_BLOCK, &cover, NULL);

	Node *temp = jobs;
	if(temp != NULL && temp -> job_id == 0) {
		log_ctrl_z();
		temp -> state = STOPPED;
		kill(temp -> pid, SIGTSTP);
	}
	sigprocmask(SIG_SETMASK, &prev, NULL);
}

/**
 * Continues the stopped process.
 */
void cont_process(int sig) {
	printf("Entered CONT signal handler.\n");
}

/**
 * This function deletes the Node with the matching PID once a process is terminated
 * along with printing the termination log depending on the type of process.
 */
void del_pid(int pid) {
	Node *temp = jobs;
	Node *prev;

	if(temp != NULL && temp -> pid == pid) {
		if(temp -> type == FOREGROUND) {
			log_job_fg_term(temp -> pid, temp -> cmdline);
			jobs = temp -> next;
			num_jobs--;
			free(temp);
			return;
		}
		else {
			log_job_bg_term(temp -> pid, temp -> cmdline);
			jobs = temp -> next;
			free(temp);
			return;
		}
	}

	while(temp != NULL && temp -> pid != pid) {
		prev = temp;
		temp = temp -> next;
	}

	if(temp == NULL) {
		return;
	}

	log_job_bg_term(temp -> pid, temp -> cmdline);
	prev -> next = temp -> next;
	free(temp);
}


/**
 * Catches all the terminated process through the SIGCHLD signal.
 */

void child_process(int sig) {
	printf("Entered SIGCHILD handle.\n");
	
	sigset_t cover, prev;
	sigemptyset(&cover);
	sigemptyset(&prev);

	sigaddset(&cover, SIGINT);
	sigaddset(&cover, SIGTSTP);
	sigaddset(&cover, SIGCHLD);
	
	int status = RUNNING;

	// Catches terminated as well as Stopped process.
	//int pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
	
	int pid = 0;
	
	//while(pid > 0) {
	while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
		// Block the signals when you are processing a signal to prevent more signal conflicts.
		// One signal at a time.
		sigprocmask(SIG_BLOCK, &cover, NULL);

		/* Catches signals of terminated process */
		if(WIFEXITED(status)) {
			printf("Entered WIFEXITED.\n");
			del_pid(pid);
		}
		/* Catches signals of stopped process */
		else if(WIFSTOPPED(status)) {
			printf("Entered WIFSTOPPED.\n");
			Node *temp = jobs;
			while(temp != NULL && temp -> pid != pid) {
				temp = temp -> next;
				break;
			}
			temp -> state = STOPPED;
			if(temp -> job_id == 0) {
				temp -> job_id = num_jobs;
			}
			if(temp -> state == STOPPED) {
				log_job_fg_stopped(temp -> pid, temp -> cmdline);
			}
		}
		/* Catches signals of continued process */
		else if(WIFCONTINUED(status)) {
			printf("Entered WIFCONTINUED.");
			Node *temp = jobs;
			while(temp != NULL && temp -> pid != pid) {
				temp = temp -> next;
			}
			if(temp -> type == FOREGROUND) {
				log_job_fg_cont(temp -> pid, temp -> cmdline);
				waitpid(temp -> pid, &temp -> state, 0);
				del_pid(temp -> pid);
			}
			else {
				temp -> type = BACKGROUND;
				log_job_bg_cont(temp -> pid, temp -> cmdline);
			}
		}
		/* Catches all other generic signals */
		else if(WIFSIGNALED(status)) {
			printf("Entered IFSIGNALED.\n");
			Node *temp = jobs;
			while(temp != NULL && temp -> pid != pid) {
				temp = temp -> next;
			}
			if(temp -> type == FOREGROUND) {
				del_pid(pid);
			}
			else {
				del_pid(pid);
			}
		}
		sigprocmask(SIG_UNBLOCK, &prev, NULL);
	}
}

/**
 * To prevent repeated initializtion of Nodes because we are dealing with linkedlist
 * so much, I decided to create a generic Node initialization function.
 */
Node *initialize_node(char *command, int pid) {
	Node *new_node = malloc(sizeof(Node));
	strcpy(new_node -> cmdline, command);
	new_node -> pid = pid;
	new_node -> state = RUNNING;
	new_node -> next = NULL;
	return new_node;
}

/**
 * All the process other than built in commands is handled here such as fork and execv.
 */
void process_jobs(char *command, char *argv[], char *argv2[], Cmd_aux *aux) {
	int pid = 0;
	int status = RUNNING;

	/*
	if(aux->control != NONE) {
	  if(aux->control == AND) {
			int child1 = -1;
			int child2 = -1;

			if((child1 = fork()) == 0){
				// Exact same as Foreground Process
				setpgid(0, 0);
				if(aux -> is_bg == 0) {
					log_start_fg(getpid(), command);
				}
				char ls[MAXLINE];
				strcpy(ls, shell_path[0]);
				strcat(ls, argv[0]);
				if(execv(ls, argv) < 0) {
					strcpy(ls, shell_path[0]);
					strcat(ls, argv[0]);
					if(execv(ls, argv) < 0) {
						log_command_error(command);
						exit(0);
					}
				}
				exit(0);
			}
			else {
				Node *new_node = initialize_node(command, child1);
				new_node -> job_id = 0;
				new_node -> type = FOREGROUND;
				if(jobs == NULL) {
					jobs = new_node;
				}
				else {
					new_node -> next = jobs;
					jobs = new_node;
				}
				waitpid(child1, &status, 0);
				if(WIFEXITED(status) && WEXITSTATUS(status) == 0) {
					if(jobs -> pid == child1 && jobs -> type == FOREGROUND) {
						log_and_list(jobs -> pid, child2, command);
						Node *temp = jobs;
						jobs = temp -> next;
						free(temp);
					}
					else {
						log_and_list(jobs -> pid, child2, command);
						Node *temp = jobs;
						jobs = temp -> next;
						free(temp);
					}
				}
				if(WEXITSTATUS(status) == 0 && (child2 = fork()) == 0) {
					log_and_list(child1, child2, command);
					char lss[MAXLINE];
					strcpy(lss, shell_path[0]);
					strcat(lss, argv2[0]);
					if(execv(lss, argv2) < 0) {
						strcpy(lss, shell_path[1]);
						strcat(lss, argv2[0]);
						if(execv(lss, argv2) < 0) {
							log_command_error(command);
							exit(0);
						}
					}
					exit(0);
				}
				else{
					// First thing we do is create two separate cases for two different parents.
					if(!(child2 == 1)) {
						Node *new_node = initialize_node(command, child2);
						new_node -> job_id = 0;
						new_node -> type = FOREGROUND;
						if(jobs == NULL) {
							jobs = new_node;
						}
						else {
							new_node -> next = jobs;
							jobs = new_node;
						}
						waitpid(child2, &status, 0);
						if(WIFEXITED(status)) {
							if(jobs -> type == FOREGROUND) {
								del_pid(jobs -> pid);
							}
							else if(jobs -> type == BACKGROUND) {
								del_pid(jobs -> pid);
							}
						}
					}
					else {
						log_and_list(child1, child2, command);
						if(jobs -> type == FOREGROUND) {
							del_pid(jobs -> pid);
						}
						else {
							del_pid(jobs -> pid);
						}
					}
				}
			}

		}
		else if(aux->control == OR){

		}

	}
	*/

	// Child Process
	if((pid = fork()) == 0) {
		setpgid(0,0);

		//Signal Handler
		ctrlc_handler();
		ctrlz_handler();

		// Foreground Process
		if(aux -> is_bg == 0) {
			log_start_fg(getpid(), command);
		}

		// File Redirection for reading only
		if(aux -> in_file != NULL) {
			int file = open(aux -> in_file, O_RDONLY, 0600);
			if(file == -1) {
				log_file_open_error(aux -> in_file);
				exit(0);
			}
			else {
				dup2(file, 0);
				close(file);
			}
		}

		// File Redirection for writing
		if(aux -> out_file != NULL) {
			int file = -1;
			if(aux -> is_append) {
				file = open(aux -> out_file, O_RDWR | O_APPEND, 0600);
			}
			else {
				file = open(aux -> out_file, O_RDWR | O_CREAT, 0600);
			}

			// Failure Occurred.
			if(file == -1) {
				log_file_open_error(aux -> out_file);
				exit(0);
			}
			else {
				dup2(file, 1);
				dup2(file, 2);
				close(file);
			}
		}

		char ls[MAXLINE];
		strcpy(ls, shell_path[0]);
		strcat(ls, argv[0]);
		if(execv(ls, argv) < 0) {
			strcpy(ls, shell_path[1]);
			strcat(ls, argv[0]);
			if(execv(ls, argv) < 0) {
				log_command_error(command);
				exit(0);
			}
		}
		exit(0);
	}
	// Parent Process
	else {
		// Parent Foreground Process
		// Adding the Foreground Process to the linked list.
		
		if(aux -> is_bg == 0) {
			num_jobs++;
			Node *new_node = initialize_node(command, pid);
			new_node -> job_id = 0;
			new_node -> type = FOREGROUND;
			if(jobs == NULL) {
				jobs = new_node;
			}
			else {
				new_node -> next = jobs;
				jobs = new_node;
			}
			if(waitpid(pid, &status, 0) > 0) {
				if(WIFEXITED(status)) {
					del_pid(pid);
				}
			}
		}
		// Parent Background Process
		else {
			log_start_bg(pid, command);
			
			// Adding the Background Process to the linked list.
			num_jobs++;
			Node *new_node = initialize_node(command, pid);
			new_node -> job_id = num_jobs;
			new_node -> type = BACKGROUND;
			if(jobs == NULL) {
				jobs = new_node;
			}
			else {
				Node *temp = jobs;
				while(temp -> next != NULL) {
					temp = temp -> next;
				}
				temp -> next = new_node;
			}
		}
	}
}
