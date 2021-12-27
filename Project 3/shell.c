/* This is the only file you should update and submit. */

/* Fill in your Name and GNumber in the following two comment fields
 * Name: Kshitiz Rimal
 * GNumber:01161557
 */

#include "logging.h"
#include "shell.h"

/* Constants */
//static const char *shell_path[] = {"./", "/usr/bin/", NULL};
static const char *shell_path[] = {"./", "/usr/bin/", NULL};
static const char *built_ins[] = 
			{"quit", "help", "kill", "jobs", "fg", "bg", NULL};
// Defined macros
#define RUNNING 0
#define FOREGROUND 1
#define BACKGROUND 2
#define STOPPED 3
#define TERMINATED 4

/*struct processNode */
struct processNode 
{
    int pid;
    int jid;
    int state;
    int inBackground;
    char command[MAXLINE];
    struct processNode *next;
};
typedef struct processNode node;

/* Feel free to define additional functions and update existing lines */
int allPids[10000];
int inBackground = 0;
int numActivePids = 0;
int numKilledPids = 0;
int exitCommand = -5;

node *head = NULL;

typedef void s_handler(int);
void displayJobs(node *);
int getFGProcess();
char* joinCommandString(char *[]);
void handleKill(char *argv[]);
void handleFG(char *argv[]);
void handleBG(char *argv[]);
void handleBGFG(char *argv[], int);
s_handler* signals(int signum, s_handler *signalHandler); 

/* This function is for the wait function */
void waitFunction(int signum) 
{ 
	 wait(NULL); 
	//pid_t pid = wait(NULL); 
    //log_job_bg_term(pid, "");
} 

/*This function handles Ctrl-c */
void handleCtrC(int signum) 
{
	//signal(SIGINT, handleCtrC);
	log_ctrl_c();
	node *tmp = head;
	while(tmp != NULL) {
		if (tmp->pid > 0) 
		{
			// If the state is foreground then terminate and kill.
			if (tmp->state == FOREGROUND) {
				tmp->state = TERMINATED;
				//printf("%d %d\n", tmp->pid, tmp->state);
				kill(tmp->pid, SIGQUIT);
				//waitpid(tmp->pid, NULL, 0);
				if (waitpid(tmp->pid, &exitCommand, 0) < 0) {
						kill(tmp->pid, SIGKILL);
				}
				break;
			}
		}
		// Else go the next node.
		tmp = tmp->next;
	}
}
/*This function handles Ctrl-z status.*/
void handleCtrZ(int signum) 
{
	// calling ctrl-z function
	log_ctrl_z();
	node *tmp = head;
	//Go to the next node until it encounters null
	while(tmp != NULL) {
		if (tmp->pid > 0) 
		{
			if (tmp->state == FOREGROUND)
				break;
		}
		tmp = tmp->next;
	}
	
	if (tmp != NULL) {
		if (tmp->pid > 0 && tmp->state != TERMINATED) {
			tmp->state = STOPPED;
			kill(tmp->pid, SIGSTOP);
			//waitpid(tmp->pid, NULL, 0); 
		}
	}
}

/* This function handles childProcess and output on the console. */
void handleChildProcess(int signum) 
{
	//signal(SIGINT, handleCtrC);
	int status = -1;
	pid_t pid;
	
	//printf("%s\n", "Hello1");
	
  	while((pid = waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED)) > 0) {

		if (WIFSIGNALED(status)) {

			//printf("%s %d\n", "WIFSIGNALED", pid);
			node *tmp = head;
			while(tmp != NULL) {
				if (tmp->pid > 0) 
				{
					if (tmp->pid == pid) {
						//printf("%s %d\n", "WIFSIGNALED", pid);
						tmp->state = TERMINATED;
						log_job_fg_term_sig(tmp->pid, tmp->command);
						break;
					}
				}
				tmp = tmp->next;
			}
			// This executes if there is a stop status 
		} else if( WIFSTOPPED(status) ){
			//printf("%s\n", "Hello4");
			//printf("%s %d\n", "WIFSTOPPED", pid);
			node *tmp = head;
			while(tmp != NULL) {
				if (tmp->pid > 0) 
				{
					if (tmp->pid == pid) {
						tmp->state = STOPPED;
						//printf("%s %d\n", "WIFSTOPPED", pid);
						break;
					}
				}
				tmp = tmp->next;
			}
			if (tmp != NULL) {
				if (tmp->state == FOREGROUND) {
					 log_job_fg_stopped(tmp->pid, tmp->command);
				}
				else if (tmp->state == BACKGROUND) {
					log_job_bg_stopped(tmp->pid, tmp->command);
				}
			}
			// This executes if there is a exit status
		} else if (WIFEXITED(status)) { 
			//printf("%s %d\n", "WIFEXITED", pid);
			node *tmp = head;
			while(tmp != NULL) {
				if (tmp->pid > 0) 
				{
					if (tmp->pid == pid) {
						//printf("%s %d %d\n", "WIFEXITED", pid, tmp->state);
						
						if (tmp->state == BACKGROUND) {
							//printf("%s %d %d\n", "WIFEXITED", pid, tmp->state);
							tmp->state = TERMINATED;
							log_job_bg_term(tmp->pid, tmp->command);
							//printf("\n");
						}
						else if (tmp->state == FOREGROUND) {
							    tmp->state = TERMINATED;
							log_job_fg_term(tmp->pid, tmp->command);
						} else {
							//tmp->state = TERMINATED;
							//log_job_fg_term(tmp->pid, tmp->command);
						}
						
						break;
					}
				}
				tmp = tmp->next;
			}

		}
	}
	
}

/*This function handles foreground and background*/
void handleBGFG(char *argv[], int bgFg) {
	int jobNum = atoi(argv[1]);
	node *tmp = head;
	while(tmp != NULL) {
		if (tmp->pid > 0) 
		{
			if (tmp->jid == jobNum) {
				break;
			}
			
		}
		tmp = tmp->next;
	}

	if (tmp == NULL) {
		log_jobid_error(jobNum);
	} else {
		if (bgFg == 1) {
			log_job_bg(tmp->pid, tmp->command);
			tmp->state = RUNNING;
			kill(tmp->pid, SIGCONT);
		} 
		if (bgFg == 0) {
			log_job_fg(tmp->pid, tmp->command);
			tmp->state = FOREGROUND;
			kill(-tmp->pid, SIGCONT);
			int status;
			do {
   				waitpid(tmp->pid, &status, WUNTRACED);
   			} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		}
	}
}
/* This handles quit fucntion */
void handleQuit(int signum) 
{
	//signal(SIGINT, handleCtrC);
	exit(1);
}

node *new_node() 
{
	node *newNode = (node *) malloc(sizeof(node));
	newNode->next = NULL;
	newNode->pid = 0;
	newNode->inBackground = 0;
	return newNode;
}

/* This inserts the node in the Linked List */
node *insert(node *head, node *newNode)
{
	if (head == NULL) 
	{
		head = newNode;
	} else 
	{
		node *tmp = head;
		while (tmp != NULL && tmp->next != NULL) 
		{
			tmp = tmp->next;
		}
		tmp->next = newNode;
	}
	return head;
}


/*This deletes the node from the Linked List */
node* delete(pid_t ID){

ID = (int)ID;
node* tmp = NULL;
if(head != NULL){
node* prev = head;
tmp = head;
tmp = tmp->next;

while(tmp != NULL){
    if(tmp->pid == ID){
        node* var = tmp->next;
        prev->next = var;
        return tmp;
    }else{
        prev = tmp;
        tmp = tmp->next;
    }

    }
}
    return tmp;
}



s_handler* signals(int signum, s_handler *signalHandler) {
	struct sigaction s_action, s_prev_action;
	s_action.sa_handler = signalHandler;
	sigemptyset(&s_action.sa_mask);
	//s_action.sa_flags = ERESTART; 
	sigaction(signum, &s_action, &s_prev_action);
	return s_prev_action.sa_handler;
}

/* This handles the kill function */
void handleKill(char *argv[]) {
	int signal = atoi(argv[1]);
	int pid = atoi(argv[2]);
	node *tmp = head;
	while(tmp != NULL) {
		if (tmp->pid == pid) 
		{
			break;
		}
		tmp = tmp->next;
	}

	//int stat;
	log_kill(signal, pid);
	if (tmp != NULL) {
		if (tmp->pid > 0) {
			tmp->state = TERMINATED;
			kill(tmp->pid, SIGQUIT);
			//wait(&stat); 
		}
	}

}


/* main */
/* The entry of your shell program */
int main() 
{
    char cmdline[MAXLINE]; /* Command line */
    pid_t pid;

    //dup2(1, 2);
    signals(SIGINT,  handleCtrC); 
    signals(SIGTSTP, handleCtrZ); 
    signals(SIGCHLD, handleChildProcess);
    signals(SIGQUIT, handleQuit); 


    /* Intial Prompt and Welcome */
    log_prompt();
    log_help();

    


    /* Shell looping here to accept user command and execute */
    while (1) {

        char *argv[MAXARGS]; /* Argument list */
        Cmd_aux aux; /* Auxilliary cmd info: check shell.h */

        //node *tmp = head;

        // int s;
        // while (tmp != NULL) 
        // {
        // 	if (tmp->pid > 0) 
        // 	{
	       //  	pid_t st = waitpid(tmp->pid, &s, WNOHANG);
	       //  	if (st != 0) 
	       //  	{
	       //  		kill(tmp->pid, SIGKILL);
	       //  		if (tmp->inBackground == 1)
	       //  		{
	       //  			log_job_bg_term(tmp->pid, tmp->command);
	       //  		} else 
	       //  		{
	       //  			log_job_fg_term(tmp->pid, tmp->command);
	       //  		}
	       //  		tmp->state = TERMINATED;
	       //  		tmp->pid = 0;
	       //  		numKilledPids++;
	       //  	}
        // 	}
        // 	tmp = tmp->next;
        // }


     //    if (numActivePids > 0) {
     //    	int s;
     //    	for (int i=0; i<numActivePids;i++) {
     //    		pid_t st = waitpid(allPids[i], &s, WNOHANG);
     //    		if (st != 0) {
     //    			kill(allPids[i], SIGKILL);
     //    			log_job_bg_term(allPids[i], "");
     //    			allPids[i] = 0;
					// numKilledPids++;
     //    		}
     //    	}
     //    }
   
	/* Print prompt */
    log_prompt();

	/* Read a line */
	// note: fgets will keep the ending '\n'
	if (fgets(cmdline, MAXLINE, stdin)==NULL)
	{
	   	if (errno == EINTR)
			continue;
	    	exit(-1); 
	}

	if (feof(stdin)) {
	    	exit(0);
	}

	if (strcmp(cmdline, "\n") == 0) {
		continue;
	}

	/* Parse command line */
    cmdline[strlen(cmdline)-1] = '\0';  /* remove trailing '\n' */
	
	if (argv == NULL) {
		continue;
	}

	char cmdUser[MAXLINE];
	strcpy(cmdUser, cmdline);

	parse(cmdline, argv, &aux);			

	/* Evaluate command */
	/* add your implementation here */
	// {"quit", "help", "kill", "jobs", "fg", "bg", NULL};
	// This calls every built_ins array
	if (strcmp(argv[0], built_ins[0]) == 0) 
	{
		log_quit();
		break;
	} else if (strcmp(argv[0], built_ins[1]) == 0) 
	{
		log_help();
	} else if (strcmp(argv[0], built_ins[2]) == 0) 
	{
		// TODO: Handle kill
		handleKill(argv);
	} 
	else if (strcmp(argv[0], built_ins[3]) == 0) 
	{
		// TODO: handle jobs
		displayJobs(head);
	} else if (strcmp(argv[0], built_ins[4]) == 0) 
	{
		// TODO: handle fg
		handleBGFG(argv, 0);
	} else if (strcmp(argv[0], built_ins[5]) == 0) 
	{
		// TODO: handle bg
		handleBGFG(argv, 1);
	} else 
	{
			sigset_t mask;
			sigemptyset(&mask);
			sigaddset(&mask, SIGCHLD);
			sigaddset(&mask, SIGINT);
			sigaddset(&mask, SIGTSTP);
			sigaddset(&mask, SIGCONT);
			sigprocmask(SIG_BLOCK, &mask, NULL);
			//background process
			if ((pid = fork()) == 0) {

				//fflush(stdout);
				//signal(SIGHUP, sighup); 
				//signal(SIGCHLD, waitFunction);
        		//signal(SIGINT, signalHandler);
        		//signal(SIGUSR1, handleCtrC); 
        		//signal(SIGQUIT, sigquit); 
				sigprocmask(SIG_UNBLOCK, &mask, NULL);
        		
				if (aux.is_bg == 1) {
					log_start_bg(getpid(), cmdUser);
					log_prompt();
					//setpgid(0, 0);
				} else {
					log_start_fg(getpid(), cmdUser);
				}

				//printf("%s\n", cmdUser);
				//This statement handles the file system. Input file
				if (aux.in_file != NULL) {
					//printf("%s\n", aux.out_file);
					//fflush(stdout);
					int f = open(aux.in_file, O_RDONLY, 0600);
					if (f ==-1) {
						log_file_open_error(aux.in_file);
						exit(0);
					} else {
						dup2(f, 0);
						//dup2(f, 2);
						close(f);
					}
				}
				// Output file
				if (aux.out_file != NULL) {
					//printf("%s\n", aux.out_file);
					//fflush(stdout);
					int f = -1;
					if (aux.is_append) {
						f = open(aux.outint _file, O_RDWR | O_APPEND, 0600);
					} else {
						f = open(aux.out_file, O_RDWR | O_CREAT, 0600);
					}

					if (f ==-1) {
							log_file_open_error(aux.out_file);
							exit(0);
					} else {
							dup2(f, 1);
							dup2(f, 2);
							close(f);
					}
				}


				char cmd[MAXLINE];
				strcpy(cmd, shell_path[0]);
				strcat(cmd, argv[0]);

				setpgid(0, 0);
				
				if (execv(cmd, argv) < 0) {
					//printf("%s\n", "here");
					strcpy(cmd, shell_path[1]);
					strcat(cmd, argv[0]);
					if (execv(cmd, argv) < 0) {
						log_command_error(cmdUser);
						exit(0);
					}
				}
				exit(0);
			} else {

				//pid_t wpid;
				//int status;
				//signal(SIGINT, signalHandler);

				numActivePids++;
				node *nNode = new_node();
				nNode->pid = pid;
				nNode->state = BACKGROUND;
				nNode->inBackground = aux.is_bg;
				nNode->jid = numActivePids;
				strcpy(nNode->command, cmdUser);
				
				//}
				 if (!aux.is_bg) {
				 	nNode->state = FOREGROUND;
				 } 

				 head = insert(head, nNode);
				 
				 sigprocmask(SIG_UNBLOCK, &mask, NULL);

				if (aux.is_bg == 0) {	
					//waitpid(pid, &exitCommand, 0);
					if (waitpid(pid, &exitCommand, 0) < 0) {
						kill(pid, SIGKILL);
					}
					node *tmp = head;
					while(tmp != NULL) {
						//
						if (tmp->pid == pid) 
						{
								//printf("%d %d %d\n", tmp->pid, pid, tmp->state);
								if (tmp->state == FOREGROUND) {
									tmp->state = TERMINATED;
									log_job_fg_term(tmp->pid, cmdUser);
								} else if (tmp->state == TERMINATED) {
									tmp->state = TERMINATED;
									log_job_fg_term_sig(tmp->pid, cmdUser);
								} else if (tmp->state == STOPPED) {
									tmp->state = STOPPED;
									log_job_fg_term_sig(tmp->pid, cmdUser);
								}
							break;
						}
						tmp = tmp->next;
					}
				
					//printf("%d\n", exitCommand);
					// while(nNode->pid == pid && nNode->state == FOREGROUND)
					// {
					//       sleep(1);	
					// }
					//log_job_fg_term(pid, cmdUser);
					// if (waitpid(pid, &status, 0) < 0) {

					// } else {
					//}
				} else {
					
					
					//waitpid(pid, &status, 0);
					//signal(SIGCHLD, waitFunction); 
					//log_job_fg_term(pid, argv[0]);
					// log_job_bg_term(pid, argv[0]);
					// do {
				 //       pid_t tpid = wait(&status);
				 //       if(tpid != pid) process_terminated(tpid);
				 //    } while(tpid != pid);
					// do {
     //  					wpid = waitpid(pid, &status, WUNTRACED);
    	// 			} while (!WIFEXITED(status) && !WIFSIGNALED(status));
				}
				//if (aux.is_bg)
				//{
					

				WEXITSTATUS(exitCommand);
				WTERMSIG(exitCommand);
			}
		}
    } 
}

char* joinCommandString(char *argv[]) {
	char *joinedStr = malloc(MAXLINE*sizeof(char));
	int i=0;

	while (argv[i] != NULL) {
		if (i==0) {
			strcpy(joinedStr, argv[i]);
		} else {
			strcat(joinedStr, " ");
			strcat(joinedStr, argv[i]);
		}
		i++;
	}
	return joinedStr;
}

/* end main */
/* Diplays the job */
void displayJobs(node *head) {
	node *tmp = head;
	int numJobs = 0;
	while(tmp != NULL) {
		if (tmp->pid > 0) 
		{
			if (tmp->state == BACKGROUND) {
				numJobs++;
			} else if (tmp->state == STOPPED) {
				numJobs++;
			}
		}
		tmp = tmp->next;
	}

	log_job_number(numJobs);
	tmp = head;
	while(tmp != NULL) {
		if (tmp->pid > 0) 
		{
			if (tmp->state == BACKGROUND) {
				log_job_details(tmp->jid, tmp->pid, "Running", tmp->command);
			} else if (tmp->state == STOPPED) {
				log_job_details(tmp->jid, tmp->pid, "Stopped", tmp->command);
			}
			
		}
		tmp = tmp->next;
	}
}

/* required function as your staring point; 
 * check shell.h for details
 */

void parse(char *cmd_line, char *argv[], Cmd_aux *aux){

	int argc;
	int bg;

	while (*cmd_line && (*cmd_line == ' ')) {
		cmd_line++;
	}

	argc = 0;

	char * token = strtok(cmd_line, " ");

	aux->out_file = NULL;
	aux->in_file = NULL;

   	while( token != NULL ) {

   	  aux->is_append = -1;
   	  
   	  if (strcmp(token, ">") == 0) {
   	  		token = strtok(NULL, " ");
   	  		aux->out_file = token;
   	  		aux->is_append = 0;
   	  }	else if (strcmp(token, ">>") == 0) {
   	  		token = strtok(NULL, " ");
   	  		aux->out_file = token;
   	  		aux->is_append = 1;
   	  }	else if (strcmp(token, "<") == 0) {
   	  		token = strtok(NULL, " ");
   	  		aux->in_file = token;
   	  }	else {
   	  		if (token != NULL ) {
   	  			argv[argc] = token;
   	  			argc++;
   	  		}
   	  }

      token = strtok(NULL, " ");
      
   	}
	
	argv[argc] = NULL;

	if ((bg = (*argv[argc-1] == '&')) != 0) {
		aux->is_bg = 1;
		argv[--argc] = NULL;
	} else {
		aux->is_bg = 0;
	}

	// for(int i=0; i<argc; i++) {
	// 	printf( "%s\n", argv[i]); 
	// }

	// printf("%s %s %d %d\n", aux->in_file, aux->out_file, aux->is_append, aux->is_bg);
}


