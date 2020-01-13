//
// Created by student on 12/7/19.
//

#ifndef HW5_DEBUG_TRADER_STRUCT_H
#define HW5_DEBUG_TRADER_STRUCT_H

#endif //HW5_DEBUG_TRADER_STRUCT_H

#include "protocol.h"
#include "trader.h"

typedef struct trader {
    funds_t balance;
    quantity_t inventory;
    quantity_t quantity;
    int fd;
    char* name;
    int count;
    pthread_mutex_t mutex;
} TRADER;

struct trader trade[MAX_TRADERS];