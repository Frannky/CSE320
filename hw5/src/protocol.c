//
// Created by student on 12/3/19.
//
#include "debug.h"
#include "protocol.h"

#include "csapp.h"
#include <time.h>

int proto_send_packet(int fd, BRS_PACKET_HEADER *hdr, void *payload){
    if(fd == -1 || hdr == NULL) return -1;
    uint16_t payload_size = 0;
    if(payload != NULL){
        payload_size = hdr->size;
    }else{
        payload_size = 0;
    }

    struct timespec time;

    clock_gettime(CLOCK_REALTIME,&time);
    hdr->timestamp_sec = time.tv_sec;
    hdr->timestamp_nsec = time.tv_nsec;
    hdr->size = htons(hdr->size);
    hdr->timestamp_nsec = htonl(hdr->timestamp_nsec);
    hdr->timestamp_sec = htonl(hdr->timestamp_sec);

    int size = sizeof(BRS_PACKET_HEADER);
    int size1 = size;
    if((rio_writen(fd, hdr, size))< 0){
        return -1;
        size = size - size1;
    }
    debug("send package: fd number is %d, header size is %d", fd, size);

    size1 = payload_size;
    size = size1;
    if(payload_size != 0){
        if((rio_writen(fd, payload, size)) < 0 ){
            return -1;
            payload_size = payload_size - size1;
        }
    }

    debug("send package: fd number is %d, payload size is %d", fd, size);

    return 0;
}

int proto_recv_packet(int fd, BRS_PACKET_HEADER *hdr, void **payloadp){
    if(fd == -1 || hdr == NULL) return -1;
    int size = sizeof(BRS_PACKET_HEADER);
    int size1 = size;

    if((size1 = Rio_readn(fd, hdr, size))<= 0){
        return -1;
        size = size - size1;
    }

    hdr->size = ntohs(hdr->size);

    hdr->timestamp_nsec = ntohl(hdr->timestamp_nsec);
    hdr->timestamp_sec = ntohl(hdr->timestamp_sec);

    debug("receive package: fd number is %d, header size is %d", fd, size);

    if(hdr->size != 0){
        char *payloadp1 = malloc(hdr->size);
        memset(payloadp1, 0, hdr->size);
        size = hdr->size;
        size1 = size;
        if((Rio_readn(fd, payloadp1, size))<=0){

            free(payloadp1);

            return -1;

            size = size - size1;
        }

        debug("send package: fd number is %d, payload size is %d", fd, size);


        *payloadp = payloadp1;
        return 0;
    }

    return 0;
}
