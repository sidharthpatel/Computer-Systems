/* Do not modify this file */
#ifndef LOGGING_H
#define LOGGING_H

void log_prompt();
void log_help();
void log_quit();
void log_parse_error(char *line); // assume not needed
void log_command_error(char *line);
void log_start_bg(int pid, char *cmd);
void log_kill_error(int pid, int sig);
void log_job_bg(int pid, char *cmd);
void log_no_bg_error();
void log_job_bg_error(); // format error, assume not needed
void log_bg_notfound_error(int job_id);
void log_fg_notfound_error(int job_id);
void log_job_fg(int pid, char *cmd);
void log_job_fg_fail(int pid, char *cmd);
void log_job_bg_fail(int pid, char *cmd);
void log_job_fg_term(int pid, char *cmd);
void log_job_fg_term_sig(int pid, char *cmd);
void log_job_fg_cont(int pid, char *cmd); // not possible?
void log_job_bg_term(int pid, char *cmd);
void log_job_bg_term_sig(int pid, char *cmd);
void log_job_bg_cont(int pid, char *cmd);
void log_job_fg_stopped(int pid, char *cmd);
void log_job_bg_stopped(int pid, char *cmd);
void log_file_open_error(char *file_name);
void log_job_number(int num_jobs);
void log_job_details(int job_id, int pid, char *state, char *cmd);

#endif /*LOGGING_H*/
