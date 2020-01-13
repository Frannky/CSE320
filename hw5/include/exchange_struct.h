//
// Created by student on 12/7/19.
//

#ifndef HW5_DEBUG_EXCHANGE_STRUCT_H
#define HW5_DEBUG_EXCHANGE_STRUCT_H

#endif //HW5_DEBUG_EXCHANGE_STRUCT_H

#include "protocol.h"
#include "trader.h"
#include "semaphore.h"
#include "csapp.h"

typedef struct orderid{
    funds_t bid;                    // Current highest bid price
    funds_t ask;
    funds_t quantity;
    orderid_t orderid;
    TRADER * trader;
    struct orderid* prev;
    struct orderid* next;
} ORDERID;

typedef struct exchange{
    funds_t last;                  // Last trade price
    funds_t highest_bid;
    funds_t highest_ask;

    ORDERID * orderid;

    int count;
    pthread_mutex_t mutex;

}EXCHANGE;

int global_variable;
sem_t semophore;
sem_t waiting;

void * matchmaking();