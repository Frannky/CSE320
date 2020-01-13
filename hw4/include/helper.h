//
// Created by student on 11/9/19.
//

#ifndef HW4_DEBUG_HELPER_H
#define HW4_DEBUG_HELPER_H

#endif //HW4_DEBUG_HELPER_H



int builtin_function(char * cmd);
pid_t Fork_Helper(void);
void print_status(int jobid);
void print_All_status();
void print_help(void);
void p_error(char *s);
void a_error(char* s);
void eval(char* cmd);
void signal_quit_handler(int signal);
void Sigemptyset_Helper(sigset_t *s);
void Sigaddset_Helper(sigset_t *s, int signal);
void Sigprocmask_Helper(int num, sigset_t *new_s, sigset_t* old_s);
void Sigfillset_Helper(sigset_t *s);
void Kill_Helper(pid_t pid,int signal);
void Setpgid_Helper(pid_t pid, pid_t pgid);
void listjobs(void);
__sighandler_t Signal_Helper(int signal, __sighandler_t sig_handler);
int signal_child_handler();
int job_get_jid(int pid);
void enable_handler(int signal);
void signal_hook(int signal);
void Killpg_Helper(pid_t pid, int signal);




struct jobs{
    pid_t pid;
    pid_t pgid;
    int jid;
    JOB_STATUS e_status;
    JOB_STATUS p_status;
    JOB_STATUS status;
    TASK * task;
    char* spectask;
};


volatile extern struct jobs jobs[MAX_JOBS];

void task_implement(struct jobs job);

volatile extern int max_job_number;
volatile extern int enable_helper;
volatile extern int enabled;
volatile extern int flag;
volatile extern int exit_helper;