/*
 * Job manager for "jobber".
 */

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

//initializing the job table
int jobs_init(void) {
    // TO BE IMPLEMENTED
    Signal_Helper(SIGCHLD,signal_hook);
    sf_set_readline_signal_hook(signal_child_handler);

    for(int i = 0; i< MAX_JOBS; i++){
        jobs[i].jid = -1;
        jobs[i].pid = -1;
        jobs[i].pgid = -1;
        jobs[i].status = -1;
        jobs[i].e_status = -1;
        jobs[i].p_status = -1;
        jobs[i].task = NULL;
        jobs[i].spectask = NULL;
    }
    enabled = 0;
    enable_helper = 0;
    max_job_number = 0;
    //abort();
    return 0;
}


// finishing a job
void jobs_fini(void) {
    // TO BE IMPLEMENTED
    int temp = 1;
    while(temp){
        temp = 0;
        int i;
        for(i = 0; i< MAX_JOBS; i++){
            if(jobs[i].status != -1){
                if(jobs[i].status != ABORTED && jobs[i].status != COMPLETED){
                    // call sigchild handler
                    // cancel all the jobs that not finish
                    job_cancel(i);
                    signal_child_handler();
                    temp = 1;
                }
            }
        }
        // call sigchild handler
        signal_child_handler();

    }
    int j;

    // expunge all the jobs which are not empty
    for(j = 0; j<MAX_JOBS; j++){
        if(jobs[j].status != -1){
            job_expunge(j);
        }
    }
    //abort();
}

//
int jobs_set_enabled(int val) {
    // TO BE IMPLEMENTED

    //restore old value
    int val1 = enabled;

    //set new value
    enabled = val;

    //abort();
    return val1;
}

// return enabled information
int jobs_get_enabled() {
    // TO BE IMPLEMENTED
    //abort();
    return enabled;
}


int job_create(char *command) {
    // TO BE IMPLEMENTED
    int plot = -1;

    char** ptr = &command;

    //find a position in the job table to be implemented
    for(int i = 0; i < MAX_JOBS;i++){
        if(jobs[i].status == -1){
            plot = i;
            break;
        }
    }

    if(plot == -1) {
        fprintf(stdout,"there is no space in the job table\n");
        return -1;
    }


    //mask signal
//    sigset_t mask_all ,mask_child;
//    Sigfillset_Helper(&mask_all);
//    Sigemptyset_Helper(&mask_child);
//    Sigaddset_Helper(&mask_child,SIGCHLD);

    //signal enable handler?????????????????????????????????
    Signal_Helper(SIGUSR1,enable_handler);
    //signal child handler

    // set up command copy
    unsigned long length1 = 1+sizeof(char) * strlen(command);
    char * copy = (char*)malloc(length1);
    memcpy(copy,command,length1);

    //parse the task
    TASK *task = parse_task(ptr);

    if(task == NULL){
        free(copy);
        return -1;
    }

    //set original job table
    jobs[plot].jid = plot;
    jobs[plot].status = NEW;
    jobs[plot].task = task;
    jobs[plot].e_status = -1;
    jobs[plot].spectask = copy;

    //create a job adn changing job status
    sf_job_create(plot);
    sf_job_status_change(jobs[plot].jid,jobs[plot].status,WAITING);
    jobs[plot].p_status = jobs[plot].status;
    jobs[plot].status = WAITING;

    // if the enabled flag is set and the max runner is less than 4 then fork and run the job
    if(enabled == 1){
        if(max_job_number < MAX_RUNNERS){
            task_implement(jobs[plot]);
            return 0;
        }

    }else{
        return 0;
    }

    //abort();
    return -1;
}

int job_expunge(int jobid) {
    // TO BE IMPLEMENTED

    //test edge cases
    if(jobid < 0 || jobid >= MAX_JOBS){
        return -1;
    }
    if(jobs[jobid].status == -1){
        return -1;
    }

    //expunge the job by setting all information to null if the status is aborted or completed
    if(jobs[jobid].status != -1){
        if(jobs[jobid].status == ABORTED || jobs[jobid].status == COMPLETED){


            jobs[jobid].jid = -1;
            jobs[jobid].pid = -1;
            jobs[jobid].pgid = -1;
            jobs[jobid].status = -1;
            jobs[jobid].p_status = -1;
            jobs[jobid].e_status = -1;
            free_task(jobs[jobid].task);
            free(jobs[jobid].spectask);
            sf_job_expunge(jobid);
            return 0;

        }
    }

    //abort();
    return -1;
}

int job_cancel(int jobid) {
    // TO BE IMPLEMENTED
    //test edge cases
    if(jobid < 0 || jobid >= MAX_JOBS){
        return -1;
    }
    if(jobs[jobid].status == -1){
        return -1;
    }
    sigset_t signal_m, signal_p;
    Sigfillset_Helper(&signal_m);
    if(jobs[jobid].status != -1){

        //cancel jobs when it is waiting
        if(jobs[jobid].status == WAITING){
//            fprintf(stdout,"job_get_pgid(jobid)  = %d",job_get_pgid(jobid));
//            fprintf(stdout,"(jobid)  = %d",jobid);
            jobs[jobid].p_status = jobs[jobid].status;
            sf_job_status_change(jobid,jobs[jobid].status,ABORTED);
            jobs[jobid].status = ABORTED;

        }
        //cancel jobs when it is running or paused
        else if(jobs[jobid].status == RUNNING || jobs[jobid].status == PAUSED){
//            fprintf(stdout,"job_get_pgid(jobid)  = %d",job_get_pgid(jobid));
//            fprintf(stdout,"(jobid)  = %d",jobid);
            Sigprocmask_Helper(SIG_BLOCK, &signal_m, &signal_p);
            Killpg_Helper(jobs[jobid].pgid,SIGKILL);
            jobs[jobid].p_status = jobs[jobid].status;
            sf_job_status_change(jobid,jobs[jobid].status,CANCELED);
            jobs[jobid].status = CANCELED;
            Sigprocmask_Helper(SIG_SETMASK, &signal_p, NULL);
        }
        return 0;
    }

    //abort();
    return -1;
}

int job_pause(int jobid) {
    // TO BE IMPLEMENTED

    //test edge cases
    if(jobid < 0 || jobid >= MAX_JOBS){
        return -1;
    }
    if(jobs[jobid].status == -1){
        return -1;
    }
    sigset_t signal_m, signal_p;
    sigfillset(&signal_m);

    if(jobs[jobid].status != -1){

        //pause job when it is running
        if(jobs[jobid].status == RUNNING){
//            fprintf(stdout,"job_get_pgid(jobid)  = %d\n",job_get_pgid(jobid));
//            fprintf(stdout,"(jobid)  = %d\n",jobid);

            Sigprocmask_Helper(SIG_BLOCK, &signal_m, &signal_p);
            sf_job_pause(jobid,job_get_pgid(jobid));
            Killpg_Helper(job_get_pgid(jobid),SIGSTOP);
            Sigprocmask_Helper(SIG_SETMASK, &signal_p, NULL);
            //abort();
            //listjobs();
            return 0;
        }
    }
    return -1;
}

int job_resume(int jobid) {
    // TO BE IMPLEMENTED
    //test edge cases
    if(jobid < 0 || jobid >= MAX_JOBS){
        return -1;
    }
    if(jobs[jobid].status == -1){
        return -1;
    }

    sigset_t signal_m, signal_p;
    Sigfillset_Helper(&signal_m);


    if(jobs[jobid].status != -1){

        //resume a job when it is paused
        if(jobs[jobid].status == PAUSED){
//            fprintf(stdout,"job_get_pgid(jobid)  = %d\n",job_get_pgid(jobid));
//            fprintf(stdout,"(jobid)  = %d\n",jobid);
            Sigprocmask_Helper(SIG_BLOCK, &signal_m, &signal_p);
            sf_job_resume(jobid,job_get_pgid(jobid));
            Killpg_Helper(job_get_pgid(jobid),SIGCONT);
            Sigprocmask_Helper(SIG_SETMASK, &signal_p, NULL);
            //abort();
            return 0;
        }
    }
    return -1;
}

// get job pgid from jobid of struct ,if jobid < 1 or cannot find return 0
int job_get_pgid(int jobid) {
    // TO BE IMPLEMENTED
    //test edge cases
    if(jobid < 0 || jobid >= MAX_JOBS){
        return -1;
    }
    if(jobs[jobid].status == -1){
        return -1;
    }

    if(jobs[jobid].status != -1){
//        fprintf(stdout,"(jobid)  = %d",jobid);
        //get the pgid information when it is running paused or canceled
        if(jobs[jobid].status == RUNNING || jobs[jobid].status == PAUSED || jobs[jobid].status == CANCELED){
            return jobs[jobid].pgid;
        }
    }
    return -1;
    //abort();
}



JOB_STATUS job_get_status(int jobid) {
    // TO BE IMPLEMENTED
    //abort();
    //test edge cases
    if(jobid < 0 || jobid >= MAX_JOBS){
        return -1;
    }
    if(jobs[jobid].status == -1){
        return -1;
    }
    //get the job status
    if(jobs[jobid].status != -1){
        return jobs[jobid].status;
    }
    return -1;
}

int job_get_result(int jobid) {
    // TO BE IMPLEMENTED
    //test edge cases
    if(jobid < 0 || jobid >= MAX_JOBS){
        return -1;
    }
    if(jobs[jobid].status == -1){
        return -1;
    }

    //get the jobe exit status
    if(jobs[jobid].status != -1){
        if(jobs[jobid].status == COMPLETED ){
            return jobs[jobid].e_status;
        }
    }
    //abort();
    return -1;
}

int job_was_canceled(int jobid) {

    // TO BE IMPLEMENTED
    //test edge cases
    if(jobid < 0 || jobid >= MAX_JOBS){
        return -1;
    }
    if(jobs[jobid].status == -1){
        return -1;
    }

    // check if the job is canceled only the case current status is aborted and pre_status is canceled
    if(jobs[jobid].status != -1){
        if(jobs[jobid].status == ABORTED && jobs[jobid].p_status == CANCELED)
        {
            return 1;
        }
    }

    //abort();
    return 0;
}

char *job_get_taskspec(int jobid) {
    // TO BE IMPLEMENTED
    //test edge cases
    if(jobid < 0 || jobid >= MAX_JOBS){
        return NULL;
    }
    if(jobs[jobid].status == -1){
        return NULL;
    }

    //get the copy of the command line
    if(jobs[jobid].status != -1){
        return jobs[jobid].spectask;
    }
    //abort();
    return NULL;
}

