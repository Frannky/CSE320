//
// Created by student on 12/8/19.
//

#include "trader.h"
#include "exchange_struct.h"
#include "trader_struct.h"
#include "server.h"
#include "debug.h"
#include "csapp.h"

void * matchmaking(){
    Pthread_detach(Pthread_self());
    info("this is matchmaking function");

    global_variable = 1;
    while(global_variable){
        debug("enter status of sleeping");
        P(&semophore);
        debug("enter status of working");

        if(global_variable == 0) break;
        if(exchange->highest_ask == 0 || exchange->highest_bid == 0){
            debug("there is no buy and sell.");

        }else{

            ORDERID * findorder = exchange->orderid;
            while(findorder != NULL){

                //for each buyer find a seller
                if(findorder->bid > 0){

                    ORDERID * findmatcher = exchange->orderid;


                    while(findmatcher != NULL){

                        if(findmatcher->ask > 0){

                            if(findorder->bid >= findmatcher->ask){

                                //number of buy less than number sell
                                if(findorder->quantity < findmatcher->quantity){


                                    trader_increase_inventory(findorder->trader,findorder->quantity);
                                    if(findorder->bid > findmatcher->ask){
                                        trader_increase_balance(findorder->trader,(findorder->bid - findmatcher->ask) * (findorder->quantity));
                                    }
                                    trader_increase_balance(findmatcher->trader,(findmatcher->ask) * (findorder->quantity));
                                    findmatcher->quantity = findmatcher->quantity - findorder->quantity;
                                    BRS_NOTIFY_INFO * status = malloc(sizeof(BRS_NOTIFY_INFO));
                                    memset(status, 0 , sizeof(BRS_NOTIFY_INFO));
                                    status->quantity = htonl(findorder->quantity);
                                    status->buyer = htonl(findorder->orderid);
                                    status->price = htonl(findmatcher->ask);
                                    status->seller = htonl(findmatcher->orderid);
                                    exchange->last = findmatcher->ask;

                                    BRS_PACKET_HEADER * header_buy = malloc(sizeof(BRS_PACKET_HEADER));
                                    memset(header_buy, 0 , sizeof(BRS_PACKET_HEADER));
                                    header_buy->type = BRS_BOUGHT_PKT;
                                    header_buy->size = sizeof(BRS_NOTIFY_INFO);

                                    trader_broadcast_packet(header_buy,status);


                                    BRS_PACKET_HEADER * header_sell = malloc(sizeof(BRS_PACKET_HEADER));
                                    memset(header_sell, 0 , sizeof(BRS_PACKET_HEADER));
                                    header_sell->type = BRS_SOLD_PKT;
                                    header_sell->size = sizeof(BRS_NOTIFY_INFO);

                                    trader_broadcast_packet(header_sell,status);


                                    BRS_PACKET_HEADER * header_trade = malloc(sizeof(BRS_PACKET_HEADER));
                                    memset(header_trade, 0 , sizeof(BRS_PACKET_HEADER));
                                    header_trade->type = BRS_TRADED_PKT;
                                    header_trade->size = sizeof(BRS_NOTIFY_INFO);

                                    trader_broadcast_packet(header_trade,status);
                                    findorder->trader->count = findorder->trader->count -1;

                                    ORDERID * prev = findorder->prev;
                                    ORDERID * next = findorder->next;

                                    if(prev == NULL && next != NULL){
                                        findorder = next;
                                        exchange->orderid = next;
                                    }else if(prev != NULL && next == NULL){
                                        free(prev->next);
                                        prev->next = NULL;
                                        free(findorder);
                                        findorder = NULL;
                                    }else if(prev != NULL && next != NULL){
                                        prev->next = next;
                                        next->prev = prev;
                                        findorder = next;
                                    }

                                }

                                //number of buy equal to number of sell
                                else if (findorder->quantity == findmatcher->quantity){

                                    trader_increase_inventory(findorder->trader,findorder->quantity);
                                    if(findorder->bid > findmatcher->ask){
                                        trader_increase_balance(findorder->trader,(findorder->bid - findmatcher->ask) * (findorder->quantity));
                                    }
                                    trader_increase_balance(findmatcher->trader,(findmatcher->ask) * (findorder->quantity));

                                    BRS_NOTIFY_INFO * status = malloc(sizeof(BRS_NOTIFY_INFO));
                                    memset(status, 0 , sizeof(BRS_NOTIFY_INFO));
                                    status->quantity = htonl(findorder->quantity);
                                    status->buyer = htonl(findorder->orderid);
                                    status->price = htonl(findmatcher->ask);
                                    status->seller = htonl(findmatcher->orderid);
                                    exchange->last = findmatcher->ask;

                                    BRS_PACKET_HEADER * header_buy = malloc(sizeof(BRS_PACKET_HEADER));
                                    memset(header_buy, 0 , sizeof(BRS_PACKET_HEADER));
                                    header_buy->type = BRS_BOUGHT_PKT;
                                    header_buy->size = sizeof(BRS_NOTIFY_INFO);

                                    trader_broadcast_packet(header_buy,status);


                                    BRS_PACKET_HEADER * header_sell = malloc(sizeof(BRS_PACKET_HEADER));
                                    memset(header_sell, 0 , sizeof(BRS_PACKET_HEADER));
                                    header_sell->type = BRS_SOLD_PKT;
                                    header_sell->size = sizeof(BRS_NOTIFY_INFO);

                                    trader_broadcast_packet(header_sell,status);


                                    BRS_PACKET_HEADER * header_trade = malloc(sizeof(BRS_PACKET_HEADER));
                                    memset(header_trade, 0 , sizeof(BRS_PACKET_HEADER));
                                    header_trade->type = BRS_TRADED_PKT;
                                    header_trade->size = sizeof(BRS_NOTIFY_INFO);

                                    trader_broadcast_packet(header_trade,status);

                                    findorder->trader->count = findorder->trader->count -1;
                                    findmatcher->trader->count = findmatcher->trader->count -1;

                                    ORDERID * prev = findorder->prev;
                                    ORDERID * next = findorder->next;

                                    if(prev == NULL && next != NULL){
                                        findorder = next;
                                        exchange->orderid = next;
                                    }else if(prev != NULL && next == NULL){
                                        free(prev->next);
                                        prev->next = NULL;
                                        free(findorder);
                                        findorder = NULL;
                                    }else if(prev != NULL && next != NULL){
                                        prev->next = next;
                                        next->prev = prev;
                                        findorder = next;
                                    }

                                    ORDERID * prev1 = findmatcher->prev;
                                    ORDERID * next1 = findmatcher->next;

                                    if(prev1 == NULL && next1 != NULL){
                                        findmatcher = next1;
                                        exchange->orderid = next1;
                                    }else if(prev1 != NULL && next1 == NULL){
                                        free(prev1->next);
                                        prev1->next = NULL;
                                        free(findmatcher);
                                        findmatcher = NULL;
                                    }else if(prev1 != NULL && next1 != NULL){
                                        prev1->next = next1;
                                        next1->prev = prev1;
                                        findmatcher = next1;
                                    }



                                }
                                //number of buy bigger than number of sell
                                else{
                                    findorder->quantity = findorder->quantity - findmatcher->quantity;
                                    trader_increase_inventory(findorder->trader,findmatcher->quantity);
                                    if(findorder->bid > findmatcher->ask){
                                        trader_increase_balance(findorder->trader,(findorder->bid - findmatcher->ask) * (findmatcher->quantity));
                                    }
                                    trader_increase_balance(findmatcher->trader,(findmatcher->ask) * (findmatcher->quantity));

                                    BRS_NOTIFY_INFO * status = malloc(sizeof(BRS_NOTIFY_INFO));
                                    memset(status, 0 , sizeof(BRS_NOTIFY_INFO));
                                    status->quantity = htonl(findmatcher->quantity);
                                    status->buyer = htonl(findorder->orderid);
                                    status->price = htonl(findmatcher->ask);
                                    status->seller = htonl(findmatcher->orderid);
                                    exchange->last = findmatcher->ask;

                                    BRS_PACKET_HEADER * header_buy = malloc(sizeof(BRS_PACKET_HEADER));
                                    memset(header_buy, 0 , sizeof(BRS_PACKET_HEADER));
                                    header_buy->type = BRS_BOUGHT_PKT;
                                    header_buy->size = sizeof(BRS_NOTIFY_INFO);

                                    trader_broadcast_packet(header_buy,status);


                                    BRS_PACKET_HEADER * header_sell = malloc(sizeof(BRS_PACKET_HEADER));
                                    memset(header_sell, 0 , sizeof(BRS_PACKET_HEADER));
                                    header_sell->type = BRS_SOLD_PKT;
                                    header_sell->size = sizeof(BRS_NOTIFY_INFO);

                                    trader_broadcast_packet(header_sell,status);


                                    BRS_PACKET_HEADER * header_trade = malloc(sizeof(BRS_PACKET_HEADER));
                                    memset(header_trade, 0 , sizeof(BRS_PACKET_HEADER));
                                    header_trade->type = BRS_TRADED_PKT;
                                    header_trade->size = sizeof(BRS_NOTIFY_INFO);

                                    trader_broadcast_packet(header_trade,status);

                                    findmatcher->trader->count = findmatcher->trader->count -1;

                                    ORDERID * prev1 = findmatcher->prev;
                                    ORDERID * next1 = findmatcher->next;

                                    if(prev1 == NULL && next1 != NULL){
                                        findmatcher = next1;
                                        exchange->orderid = next1;
                                    }else if(prev1 != NULL && next1 == NULL){
                                        free(prev1->next);
                                        prev1->next = NULL;
                                        free(findmatcher);
                                        findmatcher = NULL;
                                    }else if(prev1 != NULL && next1 != NULL){
                                        prev1->next = next1;
                                        next1->prev = prev1;
                                        findmatcher = next1;
                                    }

                                }
                            }
                        }
                        findmatcher = findmatcher->next;
                    }

                }

                //for each seller find a buyer

                findorder = findorder->next;

            }
        }

    }
    V(&waiting);
    debug("match making finished!!!!");
    return NULL;
}