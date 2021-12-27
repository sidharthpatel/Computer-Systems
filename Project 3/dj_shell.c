//hey here comes the dasskicker

/* This is the only file you should update and submit. */

/* Fill in your Name and GNumber in the following two comment fields
 * Name: Dilraj Singh
 * GNumber: G01218730
 */

#include "shell.h"
#include "parse.h"

/* Constants */
#define DEBUG 1
#define MAX 100
#define FG 0
#define BG 1
//#define RUNNING 2
#define STOPPED 3
#define TERMINATED 4

static const char *shell_path[] = { "./", "/usr/bin/", NULL };
static const char *built_ins[] = { "quit", "help", "kill",
                                   "fg", "bg", "jobs", NULL};

static char *RUNNING = "RUNNING";
static char *STOP = "STOPPED";

/*typedef enum state_enum {BG, FG, RUNNING, STOPPED, TERMINATED} State;
*/

typedef struct _node
{
    int pid;
    int jid;
    int state;
    char command[MAXLINE];
    struct _node *next;
}node;

typedef void (*__sighandler_t)(int);
typedef __sighandler_t sig_t;

/* ALL GLOBAL VARIABLES */
node *head = NULL;
node *fg_job = NULL;
int num_jid = 0;

/* Helper fucntions */
void read_cmd(char *command, char *argv[], char *argv2[], Cmd_aux *aux);
node *insertNode(node *head, node *newNode);
void add (int pid, Cmd_aux *aux ,char *command);
int max_jid(void);
void sigchld_handler(int signal);
node* get_job(int pid);
node *newNode_init();
sig_t sigaction_handler(int signal, sig_t handler);
void sigtstp_ctrlZ(int signal);
void sigint_ctrlC(int signal);
node* get_job_fg(node *head);
node* get_job_jid(int jid);
void insert_fg( node* fg_node);
void jobs_command(void);
void fg_command(char *argv[],Cmd_aux *aux);
void bg_command(char *argv[],Cmd_aux *aux);
void kill_command(char *argv[],Cmd_aux *aux);
node* delete_pid(int pid);

/*
 *
 */
void kill_command(char *argv[],Cmd_aux *aux){

    // sig and pid
    int sig = atoi(argv[1]);
    int pid = atoi(argv[2]);

    printf(" sig = %d \n pid = %d\n",sig, pid);

    if(kill(pid,sig) < 0){
        printf("ERROR in kill");
    }

    log_kill(sig,pid);
}

void bg_command(char *argv[], Cmd_aux *aux){
    int jid = 0;
    int pid = 0;
    node *job = NULL;

    print_list(head);

    jid = atoi(argv[1]);

    job = get_job_jid(jid);
    if(job == NULL) {
        log_jobid_error(jid);
        return;
    }

    job->state = BG;
    pid = job->pid;
    printf("pid = %d\n",pid);

    if(kill(pid, SIGCONT) < 0) {
        printf("ERROR in bg\n");
    }

    log_job_bg_cont(pid,job->command);

}

node* delete_pid(int pid){

    node* ret_node = NULL;

    if(head == NULL) {
        return ret_node;
    }
    else if(head->pid == pid){
        ret_node = head;
        head = head->next;
        ret_node->next = NULL;
    }else{
        node *temp = head;
        while( (temp->next != NULL) &&(temp->next->pid != pid)) {
            temp = temp->next;
        }
        ret_node = temp->next;
        if(ret_node != NULL){
            temp->next = ret_node->next;
            ret_node->next = NULL;
        }
    }
    return ret_node;

}

node* get_job_jid(int jid){
    node *null_node = NULL;
    node *temp = head;
    while(temp != NULL){
        if(temp->jid == jid) {
            return temp;
        }
        temp = temp->next;
    }

    return null_node;
}

void fg_command(char *argv[],Cmd_aux *aux) {
    int jid = 0;
    int pid = 0;
    node *job = NULL;

    jid = atoi(argv[1]);

    job = get_job_jid(jid);
    if(job == NULL) {
        log_jobid_error(jid);
        return;
    }

    job->state = FG;
    pid = job->pid;

    if(kill(-(pid),SIGCONT) < 0){
        printf("ERROR in kill command\n");
    }

    log_job_fg_cont(pid,job->command);

}


/* The function for handling Ctrl-C
 * Catches SIFINT (Ctrl-C)
 */
void sigint_ctrlC(int signal)
{
    //display the prompt
    log_ctrl_c();

    /*   node* fg_job = get_job_fg();
       int job_pid = fg_job->pid;

       if(job_pid != 0) {
           fg_job->state = TERMINATED;
           kill(-job_pid, SIGINT);
       } */

}


/* The function for handling Ctrl-Z
 * Catches SIGSTOP (Ctrl-Z)
 */
void sigtstp_ctrlZ(int signal) {
    //display the prompt
    log_ctrl_z();
    printf("inside CTRL-Z\n");
    print_list(head);

    //block all the signals received
    sigset_t mask_all, prev;
    sigfillset(&mask_all);
    sigemptyset(&prev);

    //Block all signals so that handler isnt interrupted otherwise it send a SIGCHLD
    //and is trapped in an inf loop
    sigprocmask(SIG_BLOCK,&mask_all, NULL);

    node* fg_job = get_job_fg(head);
    int job_pid = fg_job->pid;
    printf("job_pid %d\n",job_pid);

    if(job_pid > 0 )
    {
        printf("inside the cond\n");
        fg_job->state = STOPPED;
        kill(job_pid,SIGSTOP);
//    	insert_fg(fg_job);
    }

    //Unblock all the signals
    sigprocmask(SIG_SETMASK,&prev,NULL);
}

void jobs_command(void){
    int num_jobs = 0;
    node* temp = head;
    while(temp != NULL) {
        if(temp->state == BG) {
            num_jobs++;
        }
        temp = temp->next;
    }

    log_job_number(num_jobs);

    node* job = head;

    while(job != NULL) {

        //for storing the state of the process
        char *state = NULL;

        if(job->state == BG ){
            state = RUNNING;
        }
        else if( job->state == STOPPED ){
            state = STOP;
        }
        log_job_details(job->jid,job->pid,state,job->command);

        job = job->next;
    }
}

void insert_fg( node* fg_node)
{

    if(head == NULL) {
        printf("head was NULL\n");
        head = fg_node;
        return;
    }

    node *temp = head;

    while(temp != NULL && temp->next != NULL)
    {
        temp = temp->next;
    }
//	fg_node->state = 1;
    temp->next = fg_node;

}

/**
 * A handler for all the signals
 * One call and your set
 */
sig_t sigaction_handler(int signal, sig_t handler)
{
    struct sigaction new;
    struct sigaction old;

    //initializing and updating the new handler for the
    //signal we want to handle
    new.sa_handler = handler;

    // Emptying the mask so that all signals will be received
    sigemptyset(&new.sa_mask);

    if( sigaction(signal, &new, &old) < 0 )
    {
        perror("ERROR!!");
    }

    return old.sa_handler;
}


// new node init
node *newNode_init()
{
    /* Initialize new Node */
    node *newNode = malloc(sizeof(node));

    newNode->pid = 0;
    newNode->jid = 0;
    newNode->next = NULL;

    return newNode;
}

/* returns the job with the matching pid */
node* get_job(int pid)
{
    node* ret = NULL;
    node* temp = head;
    while(temp != NULL)
    {
        if( temp->pid == pid)
        {
            return temp;
        }
        temp = temp->next;
    }
    return ret;

}

/* returns the pid of the foreground job */
node* get_job_fg(node *head)
{
//    node *ret = NULL;
    node *temp = head;
    printf("inside get_job_fg\n");
    while(temp != NULL)
    {
        printf("inside loop \n");
        if(temp->state == FG)
        {
            printf("returning\n");
            return temp;
        }
        temp = temp->next;
    }
//    return ret;
}


/** CHILD HANDLER
 *
 * @param signal
 */
void sigchld_handler(int signal)
{
    //To get the status from the child
    int chld_status = 0;

    int pid = 0;

    printf("Inside SIG_CHLD\n");

    sigset_t mask_all,prev_mask;
    sigfillset(&mask_all);
    sigemptyset(&prev_mask);

    print_list(head);
    if(head == NULL)
    {printf("head is NULL\n");}
    while((pid = waitpid(-1,&chld_status, WUNTRACED|WNOHANG)) > 0) {

        //Block all signals
        sigprocmask(SIG_BLOCK, &mask_all,NULL);

        printf( "line 185 pid: %d \n", pid);

        if(WIFEXITED(chld_status)) {

            printf("if WIFEXITED \n");
            printf("pid %d\n",pid);

            print_list(head);

            node* child = get_job(pid);

            if(child->state == FG) {
                /* printf("line 188 child state is FG\n"); */
                child->state = TERMINATED;
                log_job_fg_term(child->pid, child->command);
                delete_pid(pid);
            }
            else if(child->state == BG) {
                /*  printf("line 193 child state is BG\n"); */
                printf("pid %d\n",child->pid);
                child->state = TERMINATED;
                log_job_bg_term(child->pid, child->command);
                delete_pid(pid);
            }

            /*  delete_job(pid); */

        } else if(WIFSTOPPED(chld_status)){

            printf("if WIFSTOPPED \n");

            node* child = get_job(pid);

            if(child->jid == 0)
            {
                int new_jid = num_jid + 1;
                child->jid = new_jid;
            }


//            node *newChild = malloc(sizeof(node));
//            newChild->pid = child->pid;
//            newChild->jid = child->jid;
//            newChild->state = child->state;
//            strncpy(newChild->command, child->command, MAXLINE);

            printf("child->state %d\n",child->state);
//	    printf("newchild->state %d\n",newChild->state);

            if(head == NULL) {
                printf("head is NULL\n");
            }

            // insert_fg(newChild);
            print_list(head);



            if(child->state == STOPPED) {
                log_job_fg_stopped(child->pid, child->command);
            }

        } else if(WIFSIGNALED(chld_status)) {

            printf("if WIFSIGNALED\n");

            node* child = get_job(pid);

            if(child->state == FG) {
                log_job_fg_term_sig(child->pid, child->command);
            }else if(child->state == BG) {
                log_job_bg_term_sig(child->pid, child->command);
            }

        }

        //unblock all the signals we blocked
        sigprocmask(SIG_UNBLOCK, &prev_mask,NULL);
    }

}


/*
 * inserts a node at the end of the linkedList
 */
node *insertNode(node *head, node *newNode)
{

    printf("inside insert\n");

    if(head == NULL) {
        head = newNode;
    }
    else {
        node *temp = head;
        while(temp != NULL && temp->next != NULL) {
            temp = temp->next;
        }

        /* newNode->next = temp->next; */
        temp->next = newNode;
    }

    return head;

}


/* main helper function for almost
 *
 */
void read_cmd(char *command, char *argv[], char *argv2[], Cmd_aux *aux) {

    int pid = 0;
    int job_id = 0;

    printf("inside read cmd\n");

    //Set the masks
    sigset_t mask;
    sigemptyset(&mask);

    int exitstatus = 0;
    /*
    // Mask that masks all the signal
    sigset_t mask_all = {{0}};
    sigfillset(&mask_all); */

    // Empty
    sigset_t prev;
    sigemptyset(&prev);

    // Begin masking
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTSTP);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    if(strcmp(command,built_ins[1])==0) {
        log_help();
    }
    else if(strcmp(command,built_ins[0])==0) {
        log_quit();
        printf("before free option\n");
        free_options(*command,argv,argv2,aux);
        printf("after free_options\n");
        exit(0);
    }else if(strcmp(argv[0],built_ins[2])==0) {
//        int signal = atoi(argv[1] + 1);
//        int pid = atoi(argv[2]);
//
//        log_kill(signal,pid);
        kill_command(argv,aux);
        printf("Kill done!!\n");

    }else if(strcmp(argv[0],built_ins[3])==0) {
        printf("inside fg \n");
        fg_command(argv, &aux);
    }else if(strcmp(argv[0],built_ins[4])==0) {
        printf("inside bg \n");
        bg_command(argv,&aux);
    }
    else if(strcmp(command,built_ins[5])==0) {
        printf("print number of jobs\n");
        jobs_command();
    } else{

        int child_pid = 0;


        /* CODE EXECUTED BY CHILD */
        if((child_pid = fork()) == 0){

            printf("inside child\n");

            sigaction_handler(SIGINT, SIG_DFL);
            sigaction_handler(SIGTSTP, SIG_DFL);

            //setting the grp pid
            setpgid(0, 0);

            // if there is an input file

            // if there is an output file

            if(aux->is_bg == 0)
            {
                log_start_fg(getpid(),command);
            }//else{
            //		log_start_bg(getpid(),command);
            //	    }

            // Unblock all the signals
            sigprocmask(SIG_SETMASK, &prev, NULL);


            //Running the child process
            char ls[MAXLINE];
            strcpy(ls,shell_path[0]);
            strcat(ls,argv[0]);

            /*  	printf("ls - %s\n",ls);
              printf(" command - %s\n",command);
                   printf(" argv[0] - %s argv[1] -%s argv[2] %s", argv[0],argv[1],argv[2]);
            */

            if(execv(ls,argv) < 0) {

                strcpy(ls, shell_path[1]);
                strcat(ls,argv[0]);

                /*	printf("ls - %s\n",ls);
                            printf(" command - %s\n",command);
                    printf(" argv[0] - %s argv[1] -%s argv[2] %s", argv[0],argv[1],argv[2]);
                */

                if(execv(ls,argv) < 0)
                {
                    /*printf("error in command"); */
                    log_command_error(command);
                    exit(0);
                }


            }

            //free the memory we allocated
            //free_argv(argv);

            //exit the child process
            exit(0);

        }else{ /* CODE EXECUTED BY PARENT */

            printf("inside parent\n");
//		sigaction_handler(SIGCHLD, sigchld_handler);
            if(aux->is_bg == 1) {
                num_jid = num_jid + 1;
            }
            node *newNode = newNode_init();
            newNode->pid = child_pid;

            //Background
            if (aux->is_bg == 1) {
                newNode->state = BG;
                newNode->jid = num_jid;
            } else if(aux->is_bg == 0)
//            Foreground
            {
                newNode->state = FG;
                newNode->jid = 0;
            }


            strncpy(newNode->command, command, MAXLINE);

            head = insertNode(head, newNode);

            // for foreground job


            //Background Process
            if(aux->is_bg == 1) {
                log_start_bg(child_pid,command);
            }

        }

    }

    //add(child_pid,&aux,command);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);

}


void add (int pid, Cmd_aux *aux ,char *command)
{
    if(aux->is_bg == 0) {
        num_jid = num_jid + 1;
        printf("num_jid = %d",num_jid);
    }
    node *newNode = newNode_init();
    newNode->pid = pid;

    //Background
    if(aux->is_bg == 1) {
        newNode->state = BG;
    } else if(aux->is_bg == 0)
        //Foreground
    {   newNode->state = FG;
    }
    newNode->jid = num_jid;
    strncpy(newNode->command, command, MAXLINE);
    head = insertNode(head, newNode);
}



void print_list( node *head) {
    node * temp = head;
    int i = 0;
    while( temp != NULL) {

        printf(" ->job pid : %d",temp->pid);
        printf("   job jid : %d",temp->jid);
        printf("   job command : %s",temp->command);
        printf("   job state: %d\n",temp->state);

        i++;

        temp = temp->next;
    }
    printf("size of list = %d\n", i);
}


/* The entry of your shell program */
int main() {
    char cmdline[MAXLINE];        /* Command line */
    char *cmd = NULL;

    /* Intial Prompt and Welcome */
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


            //CTRL-Z
            sigaction_handler(SIGTSTP, sigtstp_ctrlZ);
            //CTRL-C
            sigaction_handler(SIGINT, sigint_ctrlC);
            //Handle death of a child
            sigaction_handler(SIGCHLD, sigchld_handler);

            read_cmd(cmdline,argv,argv2,&aux);

        }

        //   free_options(&cmd, argv, argv2, &aux);
    }
    return 0;
}

