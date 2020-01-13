#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "jobber.h"

/*
 * "Jobber" job spooler.
 */

extern void a_error(char* s);
extern void eval(char* cmd);
int main(int argc, char *argv[])
{
    // TO BE IMPLEMENTED
    char * buf;
    int initial = jobs_init();
    if(initial){
        fprintf(stderr,"intialize value fail\n");
        exit(EXIT_FAILURE);
    }


    while(1){
        char prompt[] = "jobber> ";
        fflush(stdout);

        if((buf= sf_readline(prompt))== NULL && ferror(stdin)){
            a_error("input error");
            break;
        }
        if(feof(stdin)){
            fflush(stdout);
            exit(EXIT_SUCCESS);
        }
//        char** ptr = &buf;

        eval(buf);
        fflush(stdout);
        free(buf);
    }

    jobs_fini();

    exit(EXIT_SUCCESS);
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
