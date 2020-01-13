//
// Created by student on 12/3/19.
//
#include "client_registry.h"
#include "csapp.h"
#include "debug.h"
#include "semaphore.h"



typedef struct client_registry{
    int fd[FD_SETSIZE];
    pthread_mutex_t mutex;
    sem_t semaphore;
    int count;
    int P_flag;
} CLIENT_REGISTRY;

CLIENT_REGISTRY *creg_init(){

    CLIENT_REGISTRY *register_fd = malloc(sizeof(struct client_registry));
    memset(register_fd, 0, sizeof(struct client_registry));
    for(int i = 0; i< FD_SETSIZE ; i++){
        register_fd->fd[i] = -1;
    }
    int mutex_return = pthread_mutex_init(&register_fd->mutex, NULL);
    int semaphore_return = sem_init(&register_fd->semaphore, 0 , 0);
    register_fd->count = 0;
    register_fd->P_flag = 0;
    if(mutex_return < 0 || semaphore_return < 0){
        free(register_fd);
        return NULL;
    }
    debug("client initialize");

    return register_fd;
}

void creg_fini(CLIENT_REGISTRY *cr){
    pthread_mutex_destroy(&cr->mutex);
    sem_destroy(&cr->semaphore);
    free(cr);
    debug("client finish");

}

int creg_register(CLIENT_REGISTRY *cr, int fd){

    if(cr->fd[fd] != -1) return -1;
    else{
        pthread_mutex_lock(&(cr->mutex));
        cr->fd[fd] = fd;
        cr->count = cr->count + 1;
        debug("register: client file descriptor %d (count number: %d)", fd, cr->count);
        pthread_mutex_unlock(&(cr->mutex));
    }
    return 0;
}

int creg_unregister(CLIENT_REGISTRY *cr, int fd){
    if(cr->fd[fd] != fd) return -1;
    else{
        pthread_mutex_lock(&(cr->mutex));
        cr->fd[fd] = -1;
        cr->count = cr->count - 1;
        //close(cr->fd[fd]);
        debug("unregister: client file descriptor %d (count number: %d)", fd, cr->count);
    }

    if(cr->count == 0 && cr->P_flag == 1) {
//        cr->P_flag = 0;
        debug("unregister: increment semaphore");
        V(&cr->semaphore);
    }
    pthread_mutex_unlock(&(cr->mutex));
    return 0;
}

void creg_wait_for_empty(CLIENT_REGISTRY *cr){

//    while(1){
//        sleep(0);
//        count = 0;
//        for(int i = 0; i< FD_SETSIZE; i++){
//            if(cr->fd[i] == -1) count++;
//        }
//
//        if(count == FD_SETSIZE) return;
//    }
    cr->P_flag = 1;
    if(cr->count == 0) V(&cr->semaphore);
    debug("wait client for empty, count number is %d",  (*cr).count);

    P(&cr->semaphore);
//    cr->P_flag = 0;
}

void creg_shutdown_all(CLIENT_REGISTRY *cr){

    for(int i = 0; i < FD_SETSIZE; i++){
        if(cr->fd[i] != -1){
            close(cr->fd[i]);
            shutdown(cr->fd[i],SHUT_RD);
            creg_unregister(cr,cr->fd[i]);
        }
    }
    debug("client shutdown all, count number is %d", cr->count);

}
