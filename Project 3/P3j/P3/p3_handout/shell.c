/* This is the only file you should update and submit. */

/* Fill in your Name and GNumber in the following two comment fields
 * Name: Connor Baker
 * GNumber: G01094252
 */
#include "./shell.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Max number of allowed jobs
#define MAXJOBS 64

/** Create an enum for different jobs states we can have.
 * Enums remind me of pattern matching in Haskell, and that is a good thing.
 * We have four different states a job can be in:
 * 1. Undefined (UNDEF)
 * 2. Foreground (FG)
 * 3. Background (BG)
 * 4. Stopped (STOP)
 *
 * We use UNDEF as the catch-all for an uninitialized job.
 * There should only ever by one FG job at a time.
 * There can be many BG jobs.
 *
 * A FG job is transformed into a STOP job when the user does CTRL+Z.
 *
 * A STOP job is transformed into a FG job by the fg command.
 *
 * A STOP job is transformed into a BG job by the bg command.
 *
 * A BG job is transformed into a FG job by the fg command.
 */
enum JOB_STATES { UNDEF, FG, BG, STOP };

/**
 * More enums!
 * Represent each valid builtin command with it's own enum value!
 * We index from one so that the value 0 isn't considered a builtin (the
 * default).
 */
enum BUILTINS {
  BUILTIN_HELP = 1,
  BUILTIN_QUIT,
  BUILTIN_FG,
  BUILTIN_BG,
  BUILTIN_JOBS,
  BUILTIN_KILL
};

// Structs
/**
 * The job struct holds all the information we need for the different logging
 * functions. Its fields are:
 * 1. pid: the process ID of the job
 * 2. jid: the job ID of the job
 * 3. state: an element of the JOB_STATES enum
 * 4. command: the original command entered that created the job
 */
typedef struct job {
  int pid;
  int jid;
  int state;
  char command[MAXLINE];
} job_t;

// We can think of input as follows:
// exec [args] [< fin] [>[>] fout] [&]
// By default, fin and fout are stdin and stdout, respectively
/**
 * The cmd struct holds all the information that we need to execute a job.
 * Its fields are:
 * 1. fin: the name of the file to take input from
 * 2. fout: the name of the file to output to
 * 3. argv: an null-terminated array of arguments to send to execv
 * 4. argc: the number of arguments to send to execv (does not include the null
 * terminator)
 * 5. builtin: an element of the BUILTINS enum
 * 6. bg: a boolean representing whether to run in the background or not
 * 7. fout_flags: flags to open the output file with -- like O_APPEND vs O_TRUNC
 * fin and fout default to stdin and stdout when NULL
 */
typedef struct cmd {
  char *fin;
  char *fout;
  char **argv;
  int argc;
  int builtin;
  int bg;
  int fout_flags;
} cmd_t;

// Global variables
static char *RUNNING = "Running";
static char *STOPPED = "Stopped";
static job_t jobs[MAXJOBS] = {{0}};

// Prototypes
// TODO: Make sure all functions have a prototype and are documented
// Signal handlers
void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

// Function which installs a handler specific to a signal
// Can't name it signal because that function already exists
typedef void (*__sighandler_t)(int);
typedef __sighandler_t sig_t;
sig_t sigaction_wrapper(int sig, sig_t handler);

// Functions that deal with input
char **get_tokenized_input(const char *str, int *num_tokens);
void release_tokenized_input(char **tok, int num_tokens);
int is_builtin_command(const char *command);
cmd_t build_cmd(char **tokenized_cmd, int num_tokens);
void release_cmd(cmd_t cmd);
void builtin_bg_command(cmd_t cmd, int *jid, int *pid);
void builtin_fg_command(cmd_t cmd, int *jid, int *pid, sigset_t *prev);
void builtin_jobs_command(void);
void builtin_kill_command(cmd_t cmd);
void repl(char *command);

// Functions which handle jobs
int select_pid(job_t job);
int select_jid(job_t job);
job_t *get_job_by_selector(int (*selector)(job_t), int match);
job_t *get_job_by_pid(int pid);
job_t *get_job_by_jid(int jid);
int get_max_jid(void);
void add_job(int pid, int state, char *command);
void zero_job(job_t *job);
void delete_job_by_pid(int pid);
int get_pid_of_fg(void);

/**
 * Loops and reaps all available children.
 * @param sig required only for function prototype -- not actually used
 */
void sigchld_handler(int sig) {
  // Hold on to the status waitpid yields
  int status = 0;

  // PID of currently reaped child
  int pid = 0;

  // mask_all blocks all the signals received
  sigset_t mask_all = {{0}};
  sigfillset(&mask_all);

  // prev blocks no signals
  // We'll use this to restore our lack of blocking signals after we handle
  // the signal
  sigset_t prev = {{0}};
  sigemptyset(&prev);

  // Reap the children ;(
  while ((pid = waitpid(-1, &status, WUNTRACED | WNOHANG)) > 0) {
    // Block all the signals to make sure the handler isn't interrupted
    sigprocmask(SIG_BLOCK, &mask_all, NULL);

    // Find out which job had its state changed
    job_t *chld = get_job_by_pid(pid);

    // Make sure the job was in a valid state to begin with.
    // Ensures the if-else statements are comprehensive in pattern matching
    assert(FG == chld->state || BG == chld->state);

    if (WIFEXITED(status)) {
      // Case where the child died normally
      assert(FG == chld->state || BG == chld->state);

      // Choose the appropriate logging function and log
      void (*log)(int, char *) =
          (FG == chld->state) ? log_job_fg_term : log_job_bg_term;
      log(chld->pid, chld->command);

      // Remove the job
      delete_job_by_pid(pid);
    } else if (WIFSTOPPED(status)) {
      // Case where the child received a SIGTSTP (CTRL+Z)
      assert(FG == chld->state || BG == chld->state);

      // Handle the case where we SIGTSTP the foreground.
      // Need to handle putting it in jobs.
      if (0 == chld->jid) {
        int new_jid = get_max_jid() + 1;
        chld->jid = new_jid;
        memcpy(&jobs[new_jid], chld, sizeof(job_t));
        zero_job(&jobs[0]);
        chld = &jobs[new_jid];
      }

      // Choose the appropriate logging function and log
      void (*log)(int, char *) =
          (FG == chld->state) ? log_job_fg_stopped : log_job_bg_stopped;
      log(chld->pid, chld->command);

      chld->state = STOP;
    } else {
      // Case where the child received a SIGINT (CTRL+C)
      assert(FG == chld->state || BG == chld->state);

      // Choose the appropriate logging function and log
      void (*log)(int, char *) =
          (FG == chld->state) ? log_job_fg_term_sig : log_job_bg_term_sig;
      log(chld->pid, chld->command);

      // Remove the job
      delete_job_by_pid(pid);
    }

    // Unblock all the signals we blocked by restoring prev
    sigprocmask(SIG_SETMASK, &prev, NULL);
  }
}

/**
 * Catches SIGINT (CTRL+C).
 * Sends signal to all members of the process group.
 * @param sig required only for function prototype -- not actually used
 */
void sigint_handler(int sig) {
  int fg_pid = get_pid_of_fg();

  if (fg_pid != 0) {
    // Negative value sends the signal to everything in the process group
    kill(-fg_pid, SIGINT);
  }
}

/**
 * Catches SIGINT (CTRL+Z).
 * Sends signal to all members of the process group.
 * @param sig required only for function prototype -- not actually used
 */
void sigtstp_handler(int sig) {
  // mask_all blocks all the signals received
  sigset_t mask_all = {{0}};
  sigfillset(&mask_all);

  // prev blocks no signals
  // We'll use this to restore our lack of blocking signals after we handle
  // the signal
  sigset_t prev = {{0}};
  sigemptyset(&prev);

  // Block all the signals to make sure the handler isn't interrupted
  sigprocmask(SIG_BLOCK, &mask_all, NULL);

  int fg_pid = get_pid_of_fg();

  if (fg_pid != 0) {
    // Negative value sends the signal to everything in the process group
    kill(-fg_pid, SIGTSTP);
  }

  // Unblock all the signals we blocked by restoring prev
  sigprocmask(SIG_SETMASK, &prev, NULL);
}

/**
 * A wrapper for sigaction that installs a handler for some signal number
 * @param signum the signal number to handler
 * @param handler the signal handler for signals of type signum
 * @return the old handler
 */
sig_t sigaction_wrapper(int signum, sig_t handler) {
  struct sigaction new = {0};
  struct sigaction old = {0};

  // Update the handler for the signal we want to handle
  new.sa_handler = handler;

  // Block all signals of the type being handled
  sigemptyset(&new.sa_mask);

  if (sigaction(signum, &new, &old) < 0) {
    perror("Signal error");
  }

  return old.sa_handler;
}

/**
 * Splits a space delimited string into different tokens
 * @param str a space delimited string
 * @param num_tokens will hold the number of tokens upon return
 * @return a malloc'd array of tokens
 */
char **get_tokenized_input(const char *str, int *num_tokens) {
  char **tokenized_input = calloc(MAXARGS, sizeof(char *));
  if (NULL == tokenized_input) {
    perror("Unable to allocate memory for tokenized_input");
    exit(1);
  }

  // Hardcoded delimiter because we're just like that
  const char *delimiter = " ";
  const char *token = NULL;
  char *dup_of_token = NULL;

  int i = 0;
  for (token = strtok((char *)str, delimiter); token != NULL;
       token = strtok(NULL, delimiter), i++) {
    // Since we duplicate the string here, we need to remember to free it later
    dup_of_token = strdup(token);
    tokenized_input[i] = dup_of_token;
  }

  // Update num_tokens with the number of tokens
  *num_tokens = i;

  // Resize tokenized_input
  char **resized_tokenized_input =
      (char **)realloc(tokenized_input, i * sizeof(char *));
  if (NULL == resized_tokenized_input) {
    perror("Unable to allocate memory to resize tokenized_input");
    exit(1);
  } else {
    tokenized_input = resized_tokenized_input;
  }

  return tokenized_input;
}

/**
 * Pretty self explanatory.
 * @param tok tokenized input to be freed
 * @param num_tokens the number of tokens
 */
void release_tokenized_input(char **tok, int num_tokens) {
  for (int i = 0; i < num_tokens; ++i) {
    free(tok[i]);
  }
  free(tok);
}

/**
 * Determines whether the command passed in is a builtin command offered by our
 * shell.
 * @param command the command used
 * @return either an element of BUILTINS or zero if not a builtin command
 */
int is_builtin_command(const char *command) {
  if (strcmp("help", command) == 0) {
    return BUILTIN_HELP;
  } else if (strcmp("quit", command) == 0) {
    return BUILTIN_QUIT;
  } else if (strcmp("fg", command) == 0) {
    return BUILTIN_FG;
  } else if (strcmp("bg", command) == 0) {
    return BUILTIN_BG;
  } else if (strcmp("jobs", command) == 0) {
    return BUILTIN_JOBS;
  } else if (strcmp("kill", command) == 0) {
    return BUILTIN_KILL;
  } else {
    return 0;
  }
}

/**
 * Creates a cmd_t from the tokenized command provided as input
 * @param tokenized_cmd tokenized input from the user
 * @param num_tokens the number of tokens
 * @return a cmd_t
 */
cmd_t build_cmd(char **tokenized_cmd, int num_tokens) {
  // Check the input isn't empty
  if (NULL == tokenized_cmd || 0 >= num_tokens) {
    perror("Unable to create cmd_t from input");
    exit(1);
  }

  // Partial initialization should set everything else to zero/NULL
  cmd_t cmd = {0};

  // Initial pass to find out how much to allocate and to begin extracting back
  // matter.
  // We use strcmp instead of just checking the first character to ensure that
  // it's not a filename which includes these characters.
  int i = 0;
  while (i < num_tokens && strcmp("<", tokenized_cmd[i]) != 0 &&
         strcmp(">", tokenized_cmd[i]) != 0 &&
         strcmp(">>", tokenized_cmd[i]) != 0 &&
         strcmp("&", tokenized_cmd[i]) != 0) {
    i++;
  }

  // Make sure the input wasn't empty
  assert(0 != i);

  // Fill in the front matter
  cmd.argc = i;

  // Plus one since we need a NULL terminator for the array
  cmd.argv = (char **)calloc((unsigned long)cmd.argc + 1, sizeof(char *));

  if (NULL == cmd.argv) {
    perror("Unable to allocate memory for tok_input.");
    exit(1);
  }

  for (int j = 0; j < cmd.argc; ++j) {
    cmd.argv[j] = strdup(tokenized_cmd[j]);
  }

  // Null terminate argv
  cmd.argv[cmd.argc] = NULL;

  // Before filling in the back matter, check if the command is a builtin
  cmd.builtin = is_builtin_command(cmd.argv[0]);

  // Fill in the back matter
  if (i == num_tokens) {
    // In this case, we traversed the entirety of the input.
    // That means that there's no fin/fout, and we run in the foreground
    cmd.fin = NULL;
    cmd.fout = NULL;
    cmd.bg = 0;
  } else {
    // Each of the following cases should fall through
    // Case with < fin
    if ('<' == tokenized_cmd[i][0]) {
      i++; // Advance to next token
      assert(i < num_tokens);
      cmd.fin = strdup(tokenized_cmd[i]);
      i++; // Advance to the next token
    }

    // Case with > or >> fout
    if ('>' == tokenized_cmd[i][0]) {
      cmd.fout_flags = (unsigned int)O_WRONLY | (unsigned int)O_CREAT;
      if (0 == strcmp(">>", tokenized_cmd[i])) {
        cmd.fout_flags |= (unsigned int)O_APPEND;
      } else {
        cmd.fout_flags |= (unsigned int)O_TRUNC;
      }
      i++; // Advance to next token
      assert(i < num_tokens);
      cmd.fout = strdup(tokenized_cmd[i]);
      i++;
    }

    // Case with &
    if (i < num_tokens) {
      cmd.bg = ('&' == tokenized_cmd[i][0]);
    } else {
      cmd.bg = 0;
    }
  }

  return cmd;
}

/**
 * Frees all of the memory allocated to cmd and zeros its fields
 * @param cmd the cmd to zero out/free memory for
 */
void release_cmd(cmd_t cmd) {
  free(cmd.fin);
  free(cmd.fout);
  // Use <= to make sure we free the memory allocated for the NULL
  // terminator
  for (int i = 0; i <= cmd.argc; ++i) {
    free(cmd.argv[i]);
  }
  free(cmd.argv);
  cmd.argc = 0;
  cmd.builtin = 0;
  cmd.bg = 0;
  cmd.fout_flags = 0;
}

/**
 * Runs the bg command.
 * @param cmd the parsed input from the command line
 * @param jid the jid to update
 * @param pid the pid to update
 */
void builtin_bg_command(cmd_t cmd, int *jid, int *pid) {
  assert(cmd.argc == 2);
  assert(NULL != jid);
  assert(NULL != pid);
  *jid = atoi(cmd.argv[1]);

  if (0 == *jid) {
    // Case where there are no background tasks
    log_no_bg_error();
    return;
  } else if (*jid <= 0 || MAXJOBS <= *jid || UNDEF == jobs[*jid].state) {
    // Case where the JID is either out of bounds, or the job it is
    // referencing is not valid/not found
    log_bg_notfound_error(*jid);
    return;
  }

  // Update the state of the job
  jobs[*jid].state = BG;
  *pid = jobs[*jid].pid;

  // Continue processing the job
  if (kill(-(*pid), SIGCONT) < 0) {
    log_job_bg_fail(*pid, jobs[*jid].command);
  }

  // Print out the status of that job
  // TODO: Should I log before calling kill?
  log_job_bg(*pid, jobs[*jid].command);
  log_job_bg_cont(*pid, jobs[*jid].command);
}

/**
 * Runs the fg command.
 * @param cmd the parsed input from the command line
 * @param jid the jid to update
 * @param pid the pid to update
 * @param prev the previous blocked signals to use with sigsuspend
 */
void builtin_fg_command(cmd_t cmd, int *jid, int *pid, sigset_t *prev) {
  assert(1 == cmd.argc || 2 == cmd.argc);
  assert(NULL != jid);
  assert(NULL != pid);

  if (1 == cmd.argc) {
    // Case where we foreground the task with the largest JID
    *jid = get_max_jid();
  } else {
    // Case where we foreground the task with the specified JID
    *jid = atoi(cmd.argv[1]);
  }

  if (0 == *jid) {
    // Case where there are no background tasks
    log_no_bg_error();
    return;
  } else if (*jid < 0 || MAXJOBS <= *jid || UNDEF == jobs[*jid].state) {
    // Case where the JID is either out of bounds, or the job it is
    // referencing is not valid/not found
    log_fg_notfound_error(*jid);
    return;
  }

  // Update the state of the job
  jobs[*jid].state = FG;
  *pid = jobs[*jid].pid;

  // Continue processing the job
  if (kill(-(*pid), SIGCONT) < 0) {
    log_job_fg_fail(*pid, jobs[*jid].command);
  }

  // TODO: Should I log before calling kill?
  log_job_fg(*pid, jobs[*jid].command);

  // We're no longer in the foreground, so we wait for it to finish
  while (*pid == get_pid_of_fg()) {
    sigsuspend(prev);
  }
}

/**
 * Runs the builtin jobs command.
 */
void builtin_jobs_command(void) {
  // Find the total number of jobs
  int num_jobs = 0;
  for (int i = 0; i < MAXJOBS; ++i) {
    if (0 != jobs[i].pid) {
      ++num_jobs;
    }
  }

  log_job_number(num_jobs);

  // Print out the jobs
  for (int i = 0; i < MAXJOBS; ++i) {
    // If this job isn't valid, skip it
    if (0 == jobs[i].pid) {
      continue;
    }

    char *state = NULL;
    switch (jobs[i].state) {
    case BG:
    case FG:
      state = RUNNING;
      break;
    case STOP:
      state = STOPPED;
      break;
    case UNDEF:
    default:
      perror("Bad job state.");
      // TODO: Leak here because we didn't free cmd
      exit(1);
    }

    log_job_details(jobs[i].jid, jobs[i].pid, state, jobs[i].command);
  }
}

/**
 * Runs the builtin kill command, which is just a wrapper for kill.
 * @param cmd the parsed input from the command line
 */
void builtin_kill_command(cmd_t cmd) {
  // Plus one to drop the leading minus
  int sig = atoi(cmd.argv[1] + 1);
  int pid = atoi(cmd.argv[2]);

  if (kill(pid, sig) < 0) {
    log_kill_error(pid, sig);
  }
}

/**
 * Handles all of the core logic for the Read Eval Print Loop (REPL).
 *
 * All builtins are executed immediately and are assumed to run in the
 * foreground without IO redirection.
 *
 * External commands can run in the background or the foreground. If run in the
 * foreground, wait for them to finish.
 * @param command the user inputted command.
 */
void repl(char *command) {
  int jid = 0;
  int pid = 0;

  // Tokenize the input
  char *temp = strdup(command);
  int num_tokens = 0;
  char **tok = get_tokenized_input(temp, &num_tokens);

  // Ignore empty lines
  if (0 == num_tokens || NULL == tok[0]) {
    return;
  }

  cmd_t cmd = build_cmd(tok, num_tokens);

  // Free the memory allocated for the tokenized input since we now have cmd
  release_tokenized_input(tok, num_tokens);
  free(temp);

  // Set the different masks
  sigset_t mask = {{0}};
  sigemptyset(&mask);

  // Masks ALLLLL the signals
  sigset_t mask_all = {{0}};
  sigfillset(&mask_all);

  // Empty so that we can restore signal handling at the end
  sigset_t prev = {{0}};
  sigemptyset(&prev);

  // Begin masking
  sigaddset(&mask, SIGCHLD);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTSTP);
  sigprocmask(SIG_BLOCK, &mask, NULL);

  // Check whether we're running a builtin or running an external command
  switch (cmd.builtin) {
  case BUILTIN_HELP:
    log_help();
    break;

  case BUILTIN_QUIT:
    log_quit();
    release_cmd(cmd);
    exit(0);

  case BUILTIN_BG:
    builtin_bg_command(cmd, &jid, &pid);
    break;

  case BUILTIN_FG:
    builtin_fg_command(cmd, &jid, &pid, &prev);
    break;

  case BUILTIN_JOBS:
    builtin_jobs_command();
    break;

  case BUILTIN_KILL:
    builtin_kill_command(cmd);
    break;

  default: {
    // Continue on to external job handling
    // Keep a copy of the child pid somewhere we can use it
    int chld_pid = 0;

    // "Aw, fork." -- Eleanor, The Good Place
    if ((chld_pid = fork()) == 0) {
      // Reset the signal handling
      sigaction_wrapper(SIGINT, SIG_DFL);
      sigaction_wrapper(SIGTSTP, SIG_DFL);

      // Using zero for both sets the group id to the pid
      setpgid(0, 0);

      // Default to stdin and stdout file descriptors
      int fd_in = 0;
      int fd_out = 1;

      // Handle our input file
      if (NULL != cmd.fin) {
        fd_in = open(cmd.fin, O_RDONLY, 0600);
        if (fd_in < 0) {
          log_file_open_error(cmd.fin);
          release_cmd(cmd);
          return;
        }
        // Use dup2 to update stdin to our newly opened file
        if (dup2(fd_in, STDIN_FILENO) == -1) {
          perror("Unable to use dup2 to update stdin");
          release_cmd(cmd);
          exit(1);
        }
      }

      // Handle our output file
      if (NULL != cmd.fout) {
        fd_out = open(cmd.fout, cmd.fout_flags, 0600);
        if (fd_out < 0) {
          log_file_open_error(cmd.fout);
          release_cmd(cmd);
          return;
        }
        // Use dup2 to update stdout to our newly opened file
        if (dup2(fd_out, STDOUT_FILENO) == -1) {
          perror("Unable to use dup2 to update stdout");
          release_cmd(cmd);
          exit(1);
        }
      }

      // Unblock all the signals
      sigprocmask(SIG_SETMASK, &prev, NULL);

      // Run the child process
      if (execv(cmd.argv[0], cmd.argv) < 0) {
        log_command_error(command);
      }

      // Close the file descriptors we opened previously
      close(fd_in);
      close(fd_out);

      // Free the memory we allocated to cmd
      release_cmd(cmd);

      // Exit the child process
      exit(0);
    } else {
      // Since we blocked signals back at the beginning of this function, we
      // should be able to avoid a race with the child.
      add_job(chld_pid, cmd.bg ? BG : FG, command);

      // We have to wait for the child process to terminate if we run foreground
      if (!cmd.bg) {
        while (get_pid_of_fg() == chld_pid) {
          sigsuspend(&prev);
        }
      }

      // Log the start of the job if backgrounded.
      // Otherwise, the completion of the foreground/background task is handled
      // by the sigchld_handler which is unblocked immediately after this.
      if (cmd.bg) {
        log_start_bg(chld_pid, command);
      }
    }
  }
  }

  // Unblock all the signals
  sigprocmask(SIG_UNBLOCK, &mask, NULL);

  // Free the memory we allocated to cmd
  release_cmd(cmd);
}

/**
 * Extracts the value of pid from a job_t
 * @param job the job to extract pid from
 * @return job's PID
 */
int select_pid(job_t job) { return job.pid; }

/**
 * Extracts the value of jid from a job_t
 * @param job the job to extract jid from
 * @return job's jid
 */
int select_jid(job_t job) { return job.jid; }

/**
 * Returns the index of the job which has a field which matches the match
 * argument under the field extractor selector; otherwise -1
 * @param selector extracts a field from a job_t
 * @param match the element to match
 * @return the index of the job which has a field which matches the match
 * argument under the field extractor selector; otherwise -1
 */
job_t *get_job_by_selector(int (*selector)(job_t), int match) {
  int i = 0;
  while (i < MAXJOBS && selector(jobs[i]) != match) {
    ++i;
  }

  // Either we found it or we didn't
  return (i < MAXJOBS) ? &jobs[i] : NULL;
}

/**
 * Finds the job with the corresponding pid, or NULL if none exists
 * @param pid the pid to search for
 * @return the job with the corresponding pid, or NULL if none exists
 */
job_t *get_job_by_pid(int pid) { return get_job_by_selector(select_pid, pid); }

/**
 * Finds the job with the corresponding jid, or NULL if none exists
 * @param jid the jid to search for
 * @return the job with the corresponding jid, or NULL if none exists
 */
job_t *get_job_by_jid(int jid) { return get_job_by_selector(select_jid, jid); }

/**
 * Returns the largest JID
 * @return the largest JID
 */
int get_max_jid(void) {
  int max_jid = 0;

  for (int i = 0; i < MAXJOBS; ++i) {
    max_jid = (jobs[i].jid > max_jid) ? jobs[i].jid : max_jid;
  }

  return max_jid;
}

/**
 * Add a job to the job list
 * @param pid the PID of the job
 * @param state the state of the job
 * @param command the command that spawned the job
 */
void add_job(int pid, int state, char *command) {
  // Gotta have a valid PID
  assert(1 <= pid);

  // Find out what the next jid to use is
  int jid = 0; // Foreground always has jid zero
  if (BG == state) {
    jid = get_max_jid() + 1;
  }

  if (jid >= MAXJOBS) {
    perror("Tried to create more jobs that the max allowed.");
    exit(1);
  }

  // Make sure the job at that location is zeroed out
  assert(0 == jobs[jid].pid);
  assert(0 == jobs[jid].jid);
  assert(UNDEF == jobs[jid].state);
  assert('\0' == jobs[jid].command[0]);

  // Insert the job
  jobs[jid].jid = jid;
  jobs[jid].pid = pid;
  jobs[jid].state = state;
  strncpy(jobs[jid].command, command, MAXLINE);
}

/**
 * Clears the fields of a job
 * @param job the job to clear the fields of
 */
void zero_job(job_t *job) {
  job->pid = 0;
  job->jid = 0;
  job->state = UNDEF;
  job->command[0] = '\0';
}

/**
 * Deletes a job from jobs by looking up its PID
 * @param pid the PID of the job to remove from jobs (delete)
 */
void delete_job_by_pid(int pid) {
  // Gotta have a valid PID
  assert(1 <= pid);

  job_t *job = get_job_by_selector(select_pid, pid);

  assert(NULL != job);

  zero_job(job);
}

/**
 * Returns the PID of the foreground job, or zero if there isn't one
 * @return the PID of the foreground job, or zero if there isn't one
 */
int get_pid_of_fg(void) {
  for (int i = 0; i < MAXJOBS; i++) {
    if (jobs[i].state == FG) {
      return jobs[i].pid;
    }
  }
  return 0;
}

/**
 * Runs the shell REPL
 * @return 0 if exit successfully, nonzero otherwise
 */
int main(void) {
  char buffer[MAXLINE];

  // Install the signal handlers
  // Handle CTRL+C
  sigaction_wrapper(SIGINT, sigint_handler);

  // Handle the shell's stop command (CTRL+Z)
  sigaction_wrapper(SIGTSTP, sigtstp_handler);

  // Handle death of a child ;(
  sigaction_wrapper(SIGCHLD, sigchld_handler);

  // Run the REPL!
  while (1) {
    // Print the shell frontmatter
    log_prompt();

    // Read the input
    if (fgets(buffer, MAXLINE, stdin) == NULL) {
      if (errno == EINTR) {
        continue;
      }
      exit(-1);
    }

    // Check for the end of the file if input is scripted
    if (feof(stdin)) {
      exit(0);
    }

    // Make sure input isn't just a newline
    if ('\n' == buffer[0]) {
      continue;
    }

    // Remove trailing newline character
    char *end_of_input = strchr(buffer, '\n');
    if (NULL == end_of_input) {
      perror("No newline at end of input");
      exit(1);
    }
    *end_of_input = '\0';

    // Perform REPL
    repl(buffer);

    // Make sure we flush stdout.
    fflush(stdout);

    // Really sure.
    fflush(stdout);

    // Like, SUPER sure. (I don't know why but sometimes only the third one
    // did the trick.)
    fflush(stdout);
  }
}
