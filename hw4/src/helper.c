//
// Created by student on 11/9/19.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "jobber.h"
#include "task.h"
#include "helper.h"

#define CONVERT 48

volatile struct jobs jobs[MAX_JOBS];

volatile int max_job_number = 0;
volatile int enable_helper = 0;
volatile int enabled = 0;
volatile int flag = 0;
volatile int exit_helper = 0;

// get job id from jobid of struct ,if jobid < 1 or cannot find return 0
int job_get_jid(int pid) {
    // TO BE IMPLEMENTED
    if(pid < 1) return -1;
    for(int i = 0; i<MAX_JOBS; i++){
        if(pid == jobs[i].pid){
            return jobs[i].jid;
        }
    }
    return -1;

    //abort();
}

// if it is builtin function then we return 1 else we return 0
int builtin_function(char * cmd){

    if(cmd == NULL){
        jobs_fini();
        exit(EXIT_SUCCESS);
    }

    if(strlen(cmd) == 0){
        return 0;
    }

    //rule out invalid command
    if(*cmd == ' '){
        fprintf(stdout, "invalid format.\n");

        return 0;
    }

    //rule out extra space
    int total = strlen(cmd);
    int la = total-1;
    while(*(cmd+la) == ' '){
        *(cmd+la) = '\0';
        la--;
    }


    //calculate number of space and know how many args
    int first_space = 0;
    int s = 0;
    char * second = strchr(cmd,' ');
    if(second == NULL){
        first_space = 0;
    }else{
        s = strlen(second);
        first_space = 1;
    }
    int total1 = strlen(cmd);
    int f = total1 - s;
    int i = 0;

    //store command into command
    char command[f+1];
    for(i = 0 ; i< f ; i++){
        *(command + i) = *(cmd + i);
    }
    *(command + i) = '\0';

    char * tk = strchr(cmd,' ');
    if(tk != NULL){

        //rule out invalid format
        if(tk - cmd == 0){
            fprintf(stderr, "invalid format.\n");
        }
        else{

            while(*(tk) == ' '){
                tk++;
            }

            //spool case
            if (strcmp(command, "spool") == 0){

                if(first_space == 1){


                    if(*(tk) =='\''){
                        int last = strlen(cmd);
                        if(*(cmd+last-1) != '\''){
                            fprintf(stderr,"invalid format.\n");

                            return 0;
                        }
                        int start = 0;

                        while(*(cmd+start) != '\''){
                            start++;
                        }
                        start++;
                        while(*(cmd+start) != '\''){
                            start++;
                        }
                        if(start+1 != last){
                            fprintf(stderr,"Wrong number of args.\n");

                            return 0;
                        }

                        tk++;
                        int in = 0;
                        while(*(tk+in) != '\''){
                            in++;
                        }
                        *(tk + in) = '\0';
                        if(job_create(tk)){
                            fprintf(stderr,"Invalid format.\n");
                        }

                    }else{
//                      ptr = &(tk);

                        char * next_blank  = strchr(tk,' ');
                        if(next_blank != NULL){
                            fprintf(stderr,"Invalid format.\n");
                        }
                        else{
                            if(job_create(tk)){
                                fprintf(stderr,"Invalid format.\n");
                            }
                        }

//                  task = parse_task(ptr);
                    }
                }
                return 1;

            // pause case
            }else if(strcmp(command, "pause") == 0){

                //what parameter should pass here???????  which one is better atoi or strtol
                if(cmd+ strlen(cmd)-1 != tk){
                    fprintf(stderr,"Wrong number of args.\n");
                }else{
                    if(*tk - CONVERT >=0 && *tk -CONVERT <= MAX_JOBS){
//                        fprintf(stdout,"tk = %s\n",tk);
//                        fprintf(stdout,"cmd = %s\n", cmd);
//                        fprintf(stdout,"jid = %d\n", *tk - CONVERT);
                        int job_p = job_pause(*tk -CONVERT);
                        if(job_p !=0){
                            fprintf(stderr,"Invalid format.\n");
                        }
                    }else{
                        fprintf(stderr,"Invalid format.\n");
                    }
                }
                return 1;
            }

            //resume case
            else if (strcmp(command, "resume") == 0){

                //what parameter should pass here???????  cmd[1]
                if(cmd+ strlen(cmd)-1 != tk){
                    fprintf(stderr,"Wrong number of args.\n");
                }else{

                    if(*tk - CONVERT >=0 && *tk -CONVERT <= MAX_JOBS){
//                        fprintf(stdout,"tk = %s\n",tk);
//                        fprintf(stdout,"cmd = %s\n", cmd);
//                        fprintf(stdout,"jid = %d\n", *tk - CONVERT);
                        int job_r = job_resume(*tk -CONVERT);
                        if(job_r !=0){
                            fprintf(stderr,"Invalid format.\n");
                        }
                    }else{
                        fprintf(stderr,"Invalid format.\n");
                    }
                }
                return 1;
            }

            //expunge case
            else if(strcmp(command, "expunge") == 0){

                //what parameter should pass here???????  cmd
                if(cmd+ strlen(cmd)-1 != tk){
                    fprintf(stderr,"Wrong number of args.\n");
                }else{
                    if(*tk - CONVERT >=0 && *tk -CONVERT <= MAX_JOBS){
//                        fprintf(stdout,"tk = %s\n",tk);
//                        fprintf(stdout,"cmd = %s\n", cmd);
//                        fprintf(stdout,"jid = %d\n", *tk - CONVERT);
                        int job_e = job_expunge(*tk -CONVERT);
                        if(job_e !=0){
                            fprintf(stderr,"Invalid format.\n");
                        }
                    }else{
                        fprintf(stderr,"Invalid format.\n");
                    }
                }
                return 1;
            }

            //status case
            else if(strcmp(command, "status") == 0){
//                debug("cmd = %s", cmd);
//                debug("tk = %s", tk);
                //what parameter should pass here???????  cmd
                if(cmd+ strlen(cmd)-1 != tk){
                    fprintf(stderr,"Wrong number of args.\n");
                }else{
                    if(*tk - CONVERT >=0 && *tk -CONVERT <= MAX_JOBS){
//                        fprintf(stdout,"tk = %s\n",tk);
//                        fprintf(stdout,"cmd = %s\n", cmd);
//                        fprintf(stdout,"jid = %d\n", *tk - CONVERT);
                        int job_s = job_get_status(*tk -CONVERT);
                        if(job_s == -1){
                            fprintf(stderr,"Invalid format.\n");
                        }else{
                            fprintf(stdout,"%s %s","job", tk);
                            fprintf(stdout,"%s", "[");
                            if(job_s == ABORTED){
                                int job_c = job_was_canceled(*tk - CONVERT);
                                if(job_c == 0){
                                    fprintf(stdout,"%s %s",job_status_names[job_s], "(canceled)]: ");
                                }else{
                                    fprintf(stdout,"%s %s",job_status_names[job_s], "]: ");
                                }
                            }else if(job_s == COMPLETED){
                                fprintf(stdout,"%s %s %d %s",job_status_names[job_s], "(",job_get_result(*tk -48), ")]: ");
                            }else{
                                fprintf(stdout,"%s%s",job_status_names[job_s], "]: ");
                            }
                            fprintf(stdout,"%s\n",job_get_taskspec(*tk - CONVERT));
                        }
                    }else{
                        fprintf(stderr,"Invalid format.\n");
                    }
                }
                return 1;
            }

            //cancel case
            else if(strcmp(command, "cancel") == 0){

                //what parameter should pass here???????  cmd
                if(cmd+ strlen(cmd)-1 != tk){
                    fprintf(stderr,"Wrong number of args.\n");
                }else{
                    if(*tk - CONVERT >=0 && *tk -CONVERT <= MAX_JOBS){
//                        fprintf(stdout,"tk = %s\n",tk);
//                        fprintf(stdout,"cmd = %s\n", cmd);
//                        fprintf(stdout,"jid = %d\n", *tk - CONVERT);
                        int job_c = job_cancel(*tk -CONVERT);
                        if(job_c !=0){
                            fprintf(stderr,"Invalid format.\n");
                        }
                    }else{
                        fprintf(stderr,"Invalid format.\n");
                    }
                }
                return 1;
            }

            //other cases
            else{
                if(strcmp(command,"jobs") != 0 && strcmp(command,"enable") != 0 && strcmp(command,"disable") != 0 && strcmp(command, "help") != 0 && strcmp(command, "quit") != 0){
                    fprintf(stderr,"Wrong number of args.\n");
                }
                else{
                    fprintf(stderr,"Invalid format.\n");
                }
            }
        }

    }

    //help case
    else{
        if(strcmp(command, "help") == 0){
            print_help();
            return 1;
        }

        //quit case
        else if(strcmp(command, "quit") == 0){

            // terminate jobs that are not terminated yet
            free(cmd);
            jobs_fini();
            exit(EXIT_SUCCESS);
        }

        //enable case
        else if(strcmp(command, "enable") == 0){
            //what is the parameter of set_enable will pass??????
            // all the job status is waiting is going to be execute

            jobs_set_enabled(1);
            int j;
            for(j = 0;j < MAX_JOBS; j++){
                if(jobs[j].status != -1){
                    if(jobs[j].status == WAITING && max_job_number < MAX_RUNNERS){
                        task_implement(jobs[j]);
                    }
                }
            }

            return 1;
        }

        //disable case
        else if (strcmp(command, "disable") == 0){
            jobs_set_enabled(0);
            return 1;
        }

        //jobs case
        else if(strcmp(command, "jobs") == 0){
            int enable = jobs_get_enabled();
            if(enable != 1){
                fprintf(stdout,"%s.\n", "Job has disabled");
            }else{
                fprintf(stdout,"%s.\n", "Job has enabled");

            }
            int j;
            for(j = 0 ; j< MAX_JOBS ; j++){

                if(jobs[j].status != -1){
                    int job_s = job_get_status(j);
                    if(job_s != -1){
                        fprintf(stdout,"%s %d","job", j);
                        fprintf(stdout,"%s", "[");
                        if(job_s == ABORTED){
                            int job_c = job_was_canceled(j);
                            if(job_c == 0){
                                fprintf(stdout,"%s %s",job_status_names[job_s], "(canceled)]: ");
                            }else{
                                fprintf(stdout,"%s %s",job_status_names[job_s], "]: ");
                            }
                        }else if(job_s == COMPLETED){
                            fprintf(stdout,"%s %s %d %s",job_status_names[job_s], "(",job_get_result(j), ")]: ");
                        }else{
                            fprintf(stdout,"%s%s",job_status_names[job_s], "]: ");
                        }
                        fprintf(stdout,"%s\n",job_get_taskspec(j));
                    }
                    else{
                        fprintf(stderr,"Invalid format.\n");
                    }
                }
            }
        }

        //other cases
        else{
            if(strcmp(command,"jobs") != 0 && strcmp(command,"enable") != 0 && strcmp(command,"disable") != 0 && strcmp(command, "help") != 0 && strcmp(command, "quit") != 0){
                fprintf(stderr,"Wrong number of args.\n");
            }
            else{
                fprintf(stderr,"Invalid format.\n");
            }
        }
    }


//    task = parse_task(&cmd);
//    fprintf(stdout, "first = %s\n", task->pipelines->first->commands->first->words->first);
//    fprintf(stdout, "rest = %s\n", task->pipelines->first->commands->first->words->rest->first);
    return 0;
}

void eval(char *cmd)

{

//    char buffer[MAXLINE];
//    strcpy(buffer, cmd);
    builtin_function(cmd);
//    char* a = "echo start;cat abc | grep bash;echo end";
//    TASK *task = parse_task(&a);

}

int signal_child_handler(){

    // if global flag is not equal to 1 then return 0
    if(flag != 1){
        return 0;
    }

    //mask signal
    sigset_t signal_m, signal_p;
    pid_t pid;
    Sigfillset_Helper(&signal_m);
    int status;

    // getpgid(pid) == getpid()   J1  need to consider abort status
    // getpgid(pid) == getpid()   JOBBER   set status

//    debug("signal_child_handler(SIGCHLD) with getting child process pid = %d", getpid());

    int children_waited = 0;
    while ((pid = waitpid(-1, &status, WNOHANG | WCONTINUED | WSTOPPED)) > 0) {

        Sigprocmask_Helper(SIG_BLOCK, &signal_m, &signal_p);

//        debug("signal_child_handler with child process pid = %d", pid);
        children_waited++;
//        debug("parent pgid = %d", getpgid(pid));
//        debug("parent id = %d", parent_id);
//        debug("getpid() = %d", getpid());

        // find the postion of pid in the job table
        int plot = -1;
        for(int i = 0; i < MAX_JOBS;i++){
            if(jobs[i].status != -1){
                if(jobs[i].pid == pid){
                    plot = i;
                    break;
                }
            }
        }

//        debug("job id number = %d",plot);

        //set job table based on child process status
        if(WIFEXITED(status)){
//            debug("child process pid = %d exited $? = %d", pid, WEXITSTATUS(status));

            //when the status of child process is exited
            //set job table based on child process status as completed and decrement the max running job number
            sf_job_status_change(plot,jobs[plot].status,COMPLETED);
            jobs[plot].p_status = jobs[plot].status;
            jobs[plot].status = COMPLETED;
            jobs[plot].e_status = WEXITSTATUS(status);
            max_job_number--;
            sf_job_end(plot,jobs[plot].pgid,status);
            int i;
            for(i = 0; i< MAX_JOBS; i++){
                if(jobs[i].status != -1){
                    if(jobs[i].status == WAITING && max_job_number < MAX_RUNNERS){
                        task_implement(jobs[i]);
                    }
                }
            }

        }
        else if(WIFSTOPPED(status)){
//            debug("child process pid = %d stopped $? = %d", pid, WEXITSTATUS(status));

            //when the status of child process is stopped
            //set job table based on child process status as paused
            sf_job_status_change(plot,jobs[plot].status,PAUSED);
            jobs[plot].p_status = jobs[plot].status;
            jobs[plot].status = PAUSED;

        }
        else if(WIFSIGNALED(status)){
//            debug("child process pid = %d signaled $? = %d", pid, WEXITSTATUS(status));

            //when the status of child process is sinaled
            //set job table based on child process status as aborted and decrement the max running job number
            sf_job_status_change(plot,jobs[plot].status,ABORTED);
            jobs[plot].p_status = jobs[plot].status;
            jobs[plot].status = ABORTED;
            max_job_number--;
            sf_job_end(plot,jobs[plot].pgid,status);

            //if some job has finished and there are some jobs waiting
            //set these waiting jobs to running
            int i;
            for(i = 0; i< MAX_JOBS; i++){
                if(jobs[i].status != -1){
                    if(jobs[i].status == WAITING && max_job_number < MAX_RUNNERS){
                        task_implement(jobs[i]);
                    }
                }
            }
        }
        else if(WIFCONTINUED(status)){
//            debug("child process pid = %d signaled $? = %d", pid, WEXITSTATUS(status));

            //when the status of child process is continued
            //set job table based on child process status as running
            sf_job_status_change(plot,jobs[plot].status,RUNNING);
            jobs[plot].p_status = jobs[plot].status;
            jobs[plot].status = RUNNING;
        }
        else{

            error("child process umimplemented");
            abort();
        }

        flag = 0;
        Sigprocmask_Helper(SIG_SETMASK, &signal_p, NULL);
        }

//    if (children_waited)
//    {
////        debug("handler func successfully waited %d children", children_waited);
//    }
//    else {
////        error("handler func wait FAILED!");
//    }

    return 0;
}


//enablce handler function
void enable_handler(int signal){
    if(signal == SIGUSR1){
        enable_helper = 1;
    }
}

//fork function
pid_t Fork_Helper(void){
    pid_t pid;
    if((pid = fork()) < 0){
        fprintf(stdout,"Error happens during fork");
        abort();
    }
    return pid;
}

void print_help(void){

    printf("Available commands:\n");
    printf("help (0 args) Print this help message\n");
    printf("quit (0 args) Quit the program\n");
    printf("enable (0 args) Allow jobs to start\n");
    printf("disable (0 args) Prevent jobs from starting\n");
    printf("spool (1 args) Spool a new job\n");
    printf("pause (1 args) Pause a running job\n");
    printf("resume (1 args) Resume a paused job\n");
    printf("cancel (1 args) Cancel an unfinished job\n");
    printf("expunge (1 args) Expunge a finished job\n");
    printf("status (1 args) Print the status of a job\n");
    printf("jobs (0 args) Print the status of all jobs\n");

}

void print_status(int jobid){
    for(int i = 0; i< MAX_JOBS ; i++){
        if(jobs[i].jid == jobid){
            fprintf(stdout,"%d", jobs[i].status);
        }
    }
}

void print_All_status(){
    for(int i = 0; i< MAX_JOBS ; i++){
        fprintf(stdout,"%d", jobs[i].status);
    }
}


void signal_interrupt_handler(int signal){
    fprintf(stdout,"program terminate with control z.\n");
    flag = 1;
}

void Sigemptyset_Helper(sigset_t *s){
    if(sigemptyset(s) <0){
        p_error("signal empty set error.\n");
    }
    return;
}

void Sigaddset_Helper(sigset_t * s, int signal){
    if(sigaddset(s,signal)<0){
        p_error("signal add set error.\n");
    }
    return;
}

void Sigprocmask_Helper(int num, sigset_t *new_s, sigset_t* old_s){
    if(sigprocmask(num,new_s,old_s)<0){
        p_error("signal proc mask error.\n");
    }
    return;
}

void p_error(char *s){
    fprintf(stdout, "%s: %s\n", s, strerror(errno));
    exit(EXIT_FAILURE);
}

void a_error(char* s){
    fprintf(stdout, "%s\n", s);
    exit(EXIT_FAILURE);
}

void Sigfillset_Helper(sigset_t *s){
    if(sigfillset(s)<0){
        p_error("signal fill set error.\n");
    }
    return;
}

void Kill_Helper(pid_t pid, int signal){
    int ret;
    if((ret = kill(pid,signal)) <0){
        p_error("signal sending error.\n");

    }
}

void Killpg_Helper(pid_t pid, int signal){
    int ret;
    if((ret = killpg(pid,signal)) <0){
        p_error("signal sending error.\n");
    }
}


void Setpgid_Helper(pid_t pid, pid_t pgid){
    int ret;
    if((ret = setpgid(pid,pgid)) <0){
        p_error("set parent group id error.\n");
    }
    return;
}

__sighandler_t Signal_Helper(int signal, __sighandler_t sig_handler){
    struct sigaction action, old_action;

    action.sa_handler = sig_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;

    if (sigaction(signal, &action, &old_action) < 0)
        p_error("Signal error");
    return (old_action.sa_handler);
}

void listjobs(void) {
    int i;

    for (i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid != 0) {
            printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
            switch (jobs[i].status) {
                case NEW:
                    printf("It's a new job \n");
                    break;
                case WAITING:
                    printf("It's a waiting job \n");
                    break;
                case RUNNING:
                    printf("It's a running job \n");
                    break;
                case PAUSED:
                    printf("It's a paused job \n");
                    break;
                case CANCELED:
                    printf("It's a canceled job \n");
                    break;
                case COMPLETED:
                    printf("It's a completed job \n");
                    break;
                case ABORTED:
                    printf("It's a aborted job \n");
                    break;
                default:
                    fprintf(stdout,"listjobs: Internal error: job[%d].state=%s \n",
                           i, job_status_names[jobs[i].status]);
            }
            printf("%s", jobs[i].task->pipelines->first->commands->first->words->first);
        }
    }
}


//signal hook function
void signal_hook(int signal){
    flag = 1;
}


void task_implement(struct jobs job){


    pid_t pid;
    TASK * task = job.task;
    int plot = job.jid;

    if((pid = Fork_Helper()) == 0){

//        fprintf(stdout,"child begin!\n");

        //int count = 0;
        //set up pipeline
        PIPELINE_LIST * h = task->pipelines;
        PIPELINE_LIST * l = h;

        //consider only one command case
        if(l->rest == NULL){

            //first command
            COMMAND_LIST * list = l->first->commands;

            //rest of the command
            COMMAND_LIST* command_rest1 = l->first->commands->rest;

            //only one command case
            if(command_rest1 == NULL) {

                COMMAND *command_first = l->first->commands->first;

                int num = 0;
                WORD_LIST *first_word = command_first->words;

                //count how many words in one command
                while (first_word != NULL) {
                    num++;
                    first_word = first_word->rest;
                }
                first_word = command_first->words;
                char *argsv[num];
                //info("%d",num);

                //info("HI");
                int count2 = 0;

                //save all the words in the same command in argv
                while (first_word->rest != NULL) {
                    argsv[count2] = first_word->first;
                    first_word = first_word->rest;
                    count2++;
                }
                argsv[count2] = first_word->first;
                count2++;
                argsv[count2] = NULL;
                //info("%d",count2);


                int fdread = -1;
                int fdwrite = -1;

                //check if there has input path
                if (l->first->input_path != NULL) {
                    fdread = open(l->first->input_path, O_RDONLY);
                    if (fdread < 0) {
                        fprintf(stdout, "open file failure 1!\n");
                        abort();
                    }

                }

                //check if there has output path
                if (l->first->output_path != NULL) {
                    fdwrite = open(l->first->output_path, O_TRUNC | O_WRONLY | O_CREAT , S_IRWXU | S_IRWXG | S_IRWXO);
                    if (fdwrite < 0) {
                        fprintf(stdout, "open file failure 2!\n");
                        abort();
                    }
                }

                //redirect input path if available
                if (fdread >= 0) {
                    dup2(fdread, STDIN_FILENO);

                }
                //info("%d",fdread);

                //redirect output path if available
                if (fdwrite >= 0) {
                    dup2(fdwrite, STDOUT_FILENO);
                }
                //info("%d",fdwrite);

//                info("%s%s", argv[0], argv[1]);

                //fork child process to run the command
                if (Fork_Helper() == 0) {

                    //exeve the process
                    if (execvp(argsv[0], argsv) < 0) {
                        fprintf(stdout, "execvp file failure!\n");
                        if(fdwrite > 0) close(fdwrite);
                        if(fdread > 0) close(fdread);
                        abort();
                    }

                } else {

                    //parent process should wait child to be finished
                    if(fdwrite > 0) close(fdwrite);
                    if(fdread > 0) close(fdread);
                    int status;
                    waitpid(-1, &status, 0);
                    sigset_t signal_m,signal_p;
                    Sigfillset_Helper(&signal_m);

                    Sigprocmask_Helper(SIG_BLOCK,&signal_m,&signal_p);
                    if (WIFSIGNALED(status)) {
                        abort();
                    } else if (WIFCONTINUED(status)) { ;
                    } else if (WIFSTOPPED(status)) { ;
                    } else if (WIFEXITED(status)) {
                        //set exit status
                        exit_helper = WEXITSTATUS(status);
//                        info("%d",exit_helper);

                    } else {
                        abort();
                    }
                    Sigprocmask_Helper(SIG_SETMASK,&signal_p,NULL);
                }

                exit(exit_helper);
            }

                //if there is more than one command
            else{

                //calculate command number
                int count1 = 1;
                while(list->rest != NULL){
                    list = list->rest;
                    count1++;
                }
                //info("%d",count1);



                list = l->first->commands;
                //COMMAND * first_command = list->first;

                //calculate the pipe number
                int number_of_pipe = (count1 - 1) * 2;
                int fd[number_of_pipe];
                //info("%d",number_of_pipe);

                int* ptr1 = fd;

                //set up the pipe
                for(int i = 0; i< number_of_pipe/2 ; i++){
                    if(pipe(ptr1)<0){
                        abort();
                    }
                    ptr1 += 2;
                }
                //info("%p",ptr1);


                int pipe_count = 0;

                //fork child process to run the command
                if(Fork_Helper() == 0){
                    int fdread = -1;

                    // check input path
                    if(l->first->input_path != NULL){
                        fdread = open(l->first->input_path, O_RDONLY);
                        if(fdread < 0){
                            fprintf(stdout,"open file failure 3!\n");
                            abort();
                        }

                    }

                    //redirect the input path for the first command
                    if(fdread >= 0){

                        dup2(fdread,STDIN_FILENO);

                    }
                    //info("%d",fdread);

                    //set up the pipe for the first command
                    dup2(fd[pipe_count+1],STDOUT_FILENO);

                    //close other pipe
                    for(int i = 0; i < number_of_pipe; i++){
                        close(fd[i]);
                    }

                    //first command
                    COMMAND* command_first = list->first;

                    int num = 0;

                    //first word
                    WORD_LIST *first_word = command_first->words;

                    //count number of words in a command
                    while(first_word != NULL){
                        num++;
                        first_word = first_word->rest;
                    }
                    first_word = command_first->words;
                    char *argsv[num];
                    //info("%d",num);

                    int count2 = 0;

                    //store all the words in argv
                    while(first_word->rest != NULL){
                        argsv[count2] = first_word->first;
                        first_word = first_word->rest;
                        count2++;
                    }
                    argsv[count2] = first_word->first;
                    count2++;
                    argsv[count2] = NULL;
                    //info("%d",count2);

//                    info("%s%s",argv[0],argv[1]);

                    //exeve the process
                    if(execvp(argsv[0],argsv) < 0){
                        fprintf(stdout,"execvp file failure!\n");
                        if(fdread >0) close(fdread);
                        abort();
                    }
                }else{

                    //parent process update pipe number , command number and command list
                    pipe_count+=2;
                    count1--;
                    list = list->rest;
                    //info("%d",pipe_count;
                    //info("%d",count1);


                    //while it does not the last command fork a child to run the process
                    //here only need to consider the pipe between them and do not need to
                    //consider the input path and output path

                    while(count1 != 1){
                        if(Fork_Helper() == 0){

                            //redirect pipe in and pipe out
                            dup2(fd[pipe_count+1],STDOUT_FILENO);
                            dup2(fd[pipe_count-2],STDIN_FILENO);
                            for(int i = 0; i < number_of_pipe; i++){
                                close(fd[i]);
                            }

                            COMMAND* command_first = list->first;

                            int num = 0;
                            WORD_LIST *first_word = command_first->words;

                            //count the word number
                            while(first_word != NULL){
                                num++;
                                first_word = first_word->rest;
                            }
                            first_word = command_first->words;
                            char *argsv[num];
                            //info("%d",num);

                            int count2 = 0;

                            // save all the words in a command to argv
                            while(first_word->rest != NULL){
                                argsv[count2] = first_word->first;
                                first_word = first_word->rest;
                                count2++;
                            }
                            argsv[count2] = first_word->first;
                            count2++;
                            argsv[count2] = NULL;
                            //info("%d",count2);

                            info("%s%s",argsv[0],argsv[1]);

                            //exeve the command
                            if(execvp(argsv[0],argsv) < 0){
                                fprintf(stdout,"execvp file failure!\n");
                                abort();
                            }

                        }else{
                            //parent process update pipe number , command number and command list

                            pipe_count+=2;
                            count1--;
                            list = list->rest;
                            //info("%d",pipe_count);
                            //info("%d",count1);

                        }

                    }

                    //when count == 1 it is the last command to be execute
                    //fork a child process to run the last process
                    if(Fork_Helper() == 0) {

                        //last process may be have the output path
                        int fdwrite = -1;
                        if (l->first->output_path != NULL) {
                            fdwrite = open(l->first->output_path, O_TRUNC | O_WRONLY | O_CREAT , S_IRWXU | S_IRWXG | S_IRWXO);
                            if (fdwrite < 0) {
                                fprintf(stdout, "open file failure 7!\n");
                                abort();
                            }

                        }

                        //redirect output path to be the stdout
                        if (fdwrite >= 0) {
                            dup2(fdwrite, STDOUT_FILENO);

                        }
                        //info("%d",fdwrite);

                        //set up pipe to be the stdin
                        dup2(fd[pipe_count - 2], STDIN_FILENO);
                        for (int i = 0; i < number_of_pipe; i++) {
                            close(fd[i]);
                        }

                        COMMAND *command_first = list->first;

                        int num = 0;
                        WORD_LIST *first_word = command_first->words;

                        //count words number
                        while (first_word != NULL) {
                            num++;
                            first_word = first_word->rest;
                        }
                        first_word = command_first->words;
                        char *argsv[num];
                        //info("%d",num);

                        int count2 = 0;

                        //save all the words in a command to argv
                        while (first_word->rest != NULL) {
                            argsv[count2] = first_word->first;
                            first_word = first_word->rest;
                            count2++;
                        }
                        argsv[count2] = first_word->first;
                        count2++;
                        argsv[count2] = NULL;

                        //info("%d",count2);
//                        info("%s%s", argv[0], argv[1]);


                        //execute the last command
                        if (execvp(argsv[0], argsv) < 0) {
                            fprintf(stdout, "execvp file failure!\n");
                            if(fdwrite > 0) close(fdwrite);
                            abort();
                        }
                    }else{

                        //parent process should wait the the last command to be finished
                        for(int i =0; i<number_of_pipe ; i++){
                            close(fd[i]);
                        }
                        int status;
                        sigset_t signal_m, signal_p;
                        Sigfillset_Helper(&signal_m);
                        while(waitpid(-1, &status, WNOHANG | WCONTINUED | WSTOPPED) > 0){

                            Sigprocmask_Helper(SIG_BLOCK, &signal_m, &signal_p);
                            if(WIFSIGNALED(status)){
                                abort();
                            }
                            else if(WIFCONTINUED(status)){
                                ;
                            }
                            else if(WIFSTOPPED(status)){
                                ;
                            }
                            else if(WIFEXITED(status)){

                                //set exit status and ??????????????????????????????????
                                exit_helper = WIFEXITED(status);
                                enable_helper ++ ;
                                //debug("exit_helper is = %d", exit_helper);

                                //debug("enable_helper is = %d", enable_helper);

                            }
                            else{
                                abort();
                            }
                            Sigprocmask_Helper(SIG_SETMASK,&signal_p,NULL);
                        }

                    }
                }
            }
            exit(exit_helper);
        }
        else{

            //if there is more than one pipeline

            while(l != NULL){

                //printf("\n\nfhijweirwufsjkefhkljklvesfghjkhfsejklefhjkefhjkefhjkefshkjefkefiu\n\n");

                // first command list
                COMMAND_LIST * list = l->first->commands;

                //rest command list
                COMMAND_LIST* command_rest1 = l->first->commands->rest;

//                fprintf(stdout,"multiple pipeline with single command\n");

                //multiple pipeline with single command
                if(command_rest1 == NULL){

                    //first command
                    COMMAND* command_first = list->first;

                    int num = 0;

                    //first words
                    WORD_LIST *first_word = command_first->words;

                    //count the words number
                    while(first_word != NULL){
                        num++;
                        first_word = first_word->rest;
                    }
                    first_word = command_first->words;
                    char *argsv[num];
                    int count2 = 0;

                    //store all the words in one command to argv
                    while(first_word->rest != NULL){
                        argsv[count2] = first_word->first;
                        first_word = first_word->rest;
                        count2++;
                    }
                    argsv[count2] = first_word->first;
                    count2++;
                    argsv[count2] = NULL;

                    //info("%s%s",argv[0],argv[1]);

                    //check input and ouput path
                    int fdread = -1;
                    int fdwrite = -1;

                    //check input path
                    if(l->first->input_path != NULL){
                        fdread = open(l->first->input_path, O_RDONLY);
                        if(fdread < 0){
                            fprintf(stdout,"open file failure 5!\n");
                            close(fdread);
                            abort();
                        }

                    }

                    //check output path
                    if(l->first->output_path != NULL){
                        fdwrite = open(l->first->output_path, O_TRUNC | O_WRONLY | O_CREAT , S_IRWXU | S_IRWXG | S_IRWXO);
                        if(fdwrite < 0){
                            fprintf(stdout,"open file failure 6!\n");
                            close(fdwrite);
                            abort();
                        }
                    }

                    //redirect the input path to be stdin
                    if(fdread >= 0){
                        dup2(fdread,STDIN_FILENO);

                    }

                    //redirect the output path tp be stdout
                    if(fdwrite >= 0){
                        dup2(fdwrite,STDOUT_FILENO);
                    }

//                    info("%s%s",argv[0],argv[1]);

                    //fork child process to run the command
                    if(Fork_Helper() == 0){
                        execvp(argsv[0],argsv);
                        abort();

//                        //child process
//                        if( < 0){
//                            fprintf(stdout,"execvp file failure!\n");
//                            abort();
//                        }
                    }else{

                        //parent process should wait child process to be finished
                        int status;
                        if(fdread < 0) close(fdread);
                        if(fdwrite < 0) close(fdwrite);
                        pid_t next_pid;
                        while((next_pid = waitpid(-1, &status, WNOHANG | WCONTINUED | WSTOPPED )) > 0){


                            if(WIFSIGNALED(status)){
                                abort();
                            }
                            else if(WIFCONTINUED(status)){
                                ;
                            }
                            else if(WIFSTOPPED(status)){
                                ;
                            }
                            else if(WIFEXITED(status)){
                                //set exit status
                                exit_helper = WEXITSTATUS(status);
                                //debug("exit_helper is = %d", exit_helper);

                            }
                            else{
                                abort();
                            }
                        }
                    }


                }else{

                    // multiple command situation

                    //count the number of all the command
                    int count1 = 0;
                    while(list != NULL){
                        list = list->rest;
                        count1++;
                    }

                    list = l->first->commands;
                    //COMMAND * first_command = list->first;

                    //calculate the number of pipe among commands
                    int number_of_pipe = (count1 - 1) * 2;
                    int fd[number_of_pipe];
                    int* ptr1 = fd;

                    //set up the pipe
                    for(int i = 0; i< number_of_pipe/2 ; i++){
                        if(pipe(ptr1)<0){
                            abort();
                        }
                        ptr1 += 2;
                    }

                    int pipe_count = 0;
                    enable_helper = 0;

                    //fork a child to run the first command
                    if(Fork_Helper() == 0){
                        int fdread = -1;

                        //check input path
                        if(l->first->input_path != NULL){
                            fdread = open(l->first->input_path, O_RDONLY);
                            if(fdread < 0){
                                fprintf(stdout,"open file failure!\n");
                                abort();
                            }

                        }

                        //redirect input path to be the stdin
                        if(fdread >= 0){
                            dup2(fdread,STDIN_FILENO);

                        }

                        //redirect first pipe to be the stdout
                        dup2(fd[pipe_count+1],STDOUT_FILENO);
                        for(int i = 0; i < number_of_pipe; i++){
                            close(fd[i]);
                        }

                        //first command
                        COMMAND* command_first = list->first;

                        int num = 0;
                        //first word
                        WORD_LIST *first_word = command_first->words;

                        //count the number of words in one command
                        while(first_word != NULL){
                            num++;
                            first_word = first_word->rest;
                        }
                        first_word = command_first->words;
                        char *argsv[num];
                        int count2 = 0;

                        //store all the words in one command to argv
                        while(first_word->rest != NULL){
                            argsv[count2] = first_word->first;
                            first_word = first_word->rest;
                            count2++;

                        }
                        argsv[count2] = first_word->first;
                        count2++;
                        argsv[count2] = NULL;

//                        info("%s%s",argv[0],argv[1]);

                        //execute first command
                        if(execvp(argsv[0],argsv) < 0){
                            fprintf(stdout,"execvp file failure!\n");
                            if(fdread< 0) close(fdread);
                            abort();
                        }
                    }else{

                        //parent process update pipe number , command number and command list
                        pipe_count+=2;
                        count1--;
                        list = list->rest;

                        //if it is not the last command
                        //only consider the pipe between them and not consider the input and
                        // output path
                        while(count1 != 1){

                            //fork a child to run the process
                            if(Fork_Helper() == 0){

                                //set pipe to the stand in and stand out
                                dup2(fd[pipe_count+1],STDOUT_FILENO);
                                dup2(fd[pipe_count-2],STDIN_FILENO);

                                //close other file descriptor
                                for(int i = 0; i < number_of_pipe; i++){
                                    close(fd[i]);
                                }

                                //first command
                                COMMAND* command_first = list->first;

                                int num = 0;

                                //first word
                                WORD_LIST *first_word = command_first->words;

                                //count the number of words in one command
                                while(first_word != NULL){
                                    num++;
                                    first_word = first_word->rest;
                                }

                                first_word = command_first->words;
                                char *argsv[num];
                                int count2 = 0;

                                //store all the words in one command to argsv
                                while(first_word->rest != NULL){
                                    argsv[count2] = first_word->first;
                                    first_word = first_word->rest;
                                    count2++;
                                }
                                argsv[count2] = first_word->first;
                                count2++;
                                argsv[count2] = NULL;

//                                info("%s%s",argv[0],argv[1]);

                                //execute the process
                                if(execvp(argsv[0],argsv) < 0){
                                    fprintf(stdout,"execvp file failure!\n");
                                    abort();
                                }

                            }else{

                                //parent process update pipe number , command number and command list
                                pipe_count+=2;
                                list = list->rest;
                                count1--;
                                //info("%d",pipe_count);
                                //info("%d",count1);

                            }

                        }

                        //fork the child process to run the last command
                        if(Fork_Helper() == 0) {
                            int fdwrite = -1;

                            //last command only need to consider output path
                            if (l->first->output_path != NULL) {
                                fdwrite = open(l->first->output_path, O_TRUNC | O_WRONLY | O_CREAT , S_IRWXU | S_IRWXG | S_IRWXO);
                                if (fdwrite < 0) {
                                    fprintf(stdout, "open file failure 2!\n");
                                    abort();
                                }

                            }

                            //redirect the output to be the stdout
                            if (fdwrite >= 0) {
                                dup2(fdwrite, STDOUT_FILENO);

                            }

                            //set up the last pipe to be the stdin
                            dup2(fd[pipe_count - 2], STDIN_FILENO);
                            for (int i = 0; i < number_of_pipe; i++) {
                                close(fd[i]);
                            }

                            //first command
                            COMMAND *command_first = list->first;

                            int num = 0;
                            //first word
                            WORD_LIST *first_word = command_first->words;

                            //count word number inside a command
                            while (first_word != NULL) {
                                num++;
                                first_word = first_word->rest;
                            }
                            first_word = command_first->words;
                            char *argsv[num];
                            int count2 = 0;
                            //info("%d",num);

                            //store all the words in a command to argsv
                            while (first_word->rest != NULL) {
                                argsv[count2] = first_word->first;
                                first_word = first_word->rest;
                                count2++;
                            }
                            argsv[count2] = first_word->first;
                            count2++;
                            argsv[count2] = NULL;
                            //info("%d",count2);

//                            info("%s%s", argv[0], argv[1]);


                            //exectute process
                            if (execvp(argsv[0], argsv) < 0) {
                                fprintf(stdout, "execvp file failure!\n");
                                if(fdwrite > 0) close(fdwrite);
                                abort();
                            }
                        }else{

                            //parent process of the last command
                            // it should close all the file descriptor
                            for(int i =0; i<number_of_pipe ; i++){
                                close(fd[i]);
                            }
                            int status;
                            //pid_t next_pid;

                            //and mask all the signal to wait the child process

                            sigset_t signal_m, signal_p;
                            Sigfillset_Helper(&signal_m);
                            while(waitpid(-1, &status, WNOHANG | WCONTINUED | WSTOPPED) > 0){

                                Sigprocmask_Helper(SIG_BLOCK, &signal_m, &signal_p);
                                if(WIFSIGNALED(status)){
                                    abort();
                                }
                                else if(WIFCONTINUED(status)){
                                    ;
                                }
                                else if(WIFSTOPPED(status)){
                                    ;
                                }
                                else if(WIFEXITED(status)){

                                    // set exit status and ????????????????????????????
                                    exit_helper = WEXITSTATUS(status);
                                    //debug("exit_helper is = %d", exit_helper);
                                    enable_helper ++ ;
                                    //debug("enable_helper is = %d", enable_helper);

                                }
                                else{
                                    abort();
                                }
                                Sigprocmask_Helper(SIG_SETMASK, &signal_p, NULL);
                            }
                        }
                    }
                }

                l = l->rest;
            }
        }

        //return with exit status
        exit(exit_helper);

    }else{

        //jobber process
        sigset_t signal_m, signal_p;
        Sigfillset_Helper(&signal_m);

        //mask all the signal
        Sigprocmask_Helper(SIG_BLOCK,&signal_m ,&signal_p);

        //set job table pid
        jobs[plot].pid = pid;
        if(setpgid(pid,pid) == 0){
            jobs[plot].pgid = pid;
        }else{
            jobs[plot].pgid = -1;
        }

//        debug("This is the parent with child pid = %d", pid);


        //change pid job status to running
        max_job_number++;
        //info("%d",max_job_number);

        jobs[plot].p_status = jobs[plot].status;
        sf_job_status_change(jobs[plot].jid,jobs[plot].status,RUNNING);
        jobs[plot].status = RUNNING;

        sf_job_start(jobs[plot].jid,jobs[plot].pgid);
        Sigprocmask_Helper(SIG_SETMASK, &signal_p, NULL);
    }
}