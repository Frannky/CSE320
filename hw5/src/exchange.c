//
// Created by student on 12/7/19.
//

#include <trader_struct.h>
#include "exchange.h"
#include "debug.h"
#include "csapp.h"
#include "server.h"
#include "exchange_struct.h"



/*
 * Initialize a new exchange.
 *
 * @return  the newly initialized exchange, or NULL if initialization failed.
 */
EXCHANGE *exchange_init(){

    EXCHANGE *exchange = malloc(sizeof(EXCHANGE));
    if(exchange == NULL) return NULL;
    memset(exchange, 0 , sizeof(EXCHANGE));

    exchange->last = 0;
    exchange->count = -1;
    pthread_mutex_init(&exchange->mutex,NULL);
    sem_init(&semophore,0,0);
    sem_init(&waiting,0,0);
    exchange->orderid = malloc(sizeof(struct orderid));
    memset(exchange->orderid, 0, sizeof(struct orderid));
    exchange->orderid->orderid = 0;
    exchange->orderid->bid = 0;
    exchange->orderid->ask = 0;
    exchange->orderid->quantity = 0;
    exchange->orderid->trader = NULL;
    exchange->orderid->next = NULL;
    exchange->orderid->prev = NULL;
    pthread_t tid;

    pthread_create(&tid,NULL,matchmaking,NULL);
    debug("exchange initialized");
    return exchange;
}

/*
 * Finalize an exchange, freeing all associated resources.
 *
 * @param xchg  The exchange to be finalized, which must not
 * be referenced again.
 */
void exchange_fini(EXCHANGE *xchg){

    xchg->last = 0;
    xchg->count = -1;
    pthread_mutex_destroy(&xchg->mutex);
    ORDERID * cursor = xchg->orderid;
    while(cursor != NULL){
        cursor->orderid = 0;
        cursor->bid = 0;
        cursor->ask = 0;
        cursor->quantity = 0;
        cursor = cursor->next;
    }
    free(xchg->orderid);
    free(cursor);
    free(xchg);
    global_variable = 0;
    V(&semophore);
    P(&waiting);
    debug("exchange finished");

    return;
}

/*
 * Get the current status of the exchange.
 */
void exchange_get_status(EXCHANGE *xchg, BRS_STATUS_INFO *infop){


//    infop->orderid = htonl(xchg->orderid->orderid);
    infop->ask = htonl(xchg->highest_ask);
    infop->bid = htonl(xchg->highest_bid);
    infop->last = htonl(xchg->last);
    debug("exchange get status");
    return;
}

/*
 * This function attempts to post a buy order on the exchange on behalf of a
specified trader.  The trader's account balance is debited by the maximum possible
price of a trade made for this order.  A reference to the TRADER object is retained
as part of the order; to prevent the TRADER object from being freed as long as
the order is pending, the reference count on the trader is increased by one when
the order is created and decreased by one when the order is eventually removed from the
exchange.  A POSTED packet containing the parameters of the posted order
is broadcast to all logged-in traders.
 *
 *
 * Post a buy order on the exchange on behalf of a trader.
 * The trader is stored with the order, and its reference count is
 * increased by one to account for the stored pointer.
 * Funds equal to the maximum possible cost of the order are
 * encumbered by removing them from the trader's account.
 * A POSTED packet containing details of the order is broadcast
 * to all logged-in traders.
 *
 * @param xchg  The exchange to which the order is to be posted.
 * @param trader  The trader on whose behalf the order is to be posted.
 * @param quantity  The quantity to be bought.
 * @param price  The maximum price to be paid per unit.
 * @return  The order ID assigned to the new order, if successfully posted,
 * otherwise 0.
 */
orderid_t exchange_post_buy(EXCHANGE *xchg, TRADER *trader, quantity_t quantity,
                            funds_t price){
    pthread_mutex_lock(&xchg->mutex);

    int isEnough = -1;
    if(trader->balance >= quantity * price){
        isEnough = 1;
    }else{
        isEnough = 0;
    }

    if(isEnough){
        trader_decrease_balance(trader, quantity * price);

        xchg->count = xchg->count + 1;
        trader->count = trader->count +1;
        if(price > xchg->highest_bid){
            xchg->highest_bid = price;
        }
        ORDERID * new_orderid = malloc(sizeof(ORDERID));
        memset(new_orderid, 0 , sizeof(ORDERID));

        new_orderid->orderid = xchg->orderid->orderid + 1;
        new_orderid->bid = price;
        new_orderid->quantity = quantity;
        new_orderid->trader = trader;

        ORDERID * head = xchg->orderid;

        xchg->orderid = new_orderid;
        new_orderid->next = head;
        head->prev = new_orderid;
        BRS_PACKET_HEADER * header = malloc(sizeof(BRS_PACKET_HEADER));
        memset(header, 0 , sizeof(BRS_PACKET_HEADER));
        BRS_NOTIFY_INFO * status = malloc(sizeof(BRS_NOTIFY_INFO));
        memset(status, 0 , sizeof(BRS_NOTIFY_INFO));
        status->quantity = htonl(quantity);
        status->buyer = htonl(new_orderid->orderid);
        status->price = htonl(price);
        status->seller = htonl(0);

        header->size = sizeof(BRS_NOTIFY_INFO);
        header->type = BRS_POSTED_PKT;

        trader_broadcast_packet(header,status);
        free(header);
        free(status);

        V(&semophore);

        pthread_mutex_unlock(&xchg->mutex);
        return new_orderid->orderid;

    }else{

        pthread_mutex_unlock(&xchg->mutex);
        return 0;
    }


}

/*
 *
 * This function attempts to post a sell order on the exchange on behalf of a
specified trader.  The trader's inventory is decreased by the specified quantity.
A reference to the TRADER object is retained as part of the order; to prevent the TRADER
object from being freed as long as the order is pending, the reference count on the trader
is increased by one when the order is created and decreased by one when the order is eventually
removed from the exchange.  A POSTED packet containing the parameters of the posted order
is broadcast to all logged-in traders.
 *
 *
 * Post a sell order on the exchange on behalf of a trader.
 * The trader is stored with the order, and its reference count is
 * increased by one to account for the stored pointer.
 * Inventory equal to the amount of the order is
 * encumbered by removing it from the trader's account.
 * A POSTED packet containing details of the order is broadcast
 * to all logged-in traders.
 *
 * @param xchg  The exchange to which the order is to be posted.
 * @param trader  The trader on whose behalf the order is to be posted.
 * @param quantity  The quantity to be sold.
 * @param price  The minimum sale price per unit.
 * @return  The order ID assigned to the new order, if successfully posted,
 * otherwise 0.
 */
orderid_t exchange_post_sell(EXCHANGE *xchg, TRADER *trader, quantity_t quantity,
                             funds_t price){
    pthread_mutex_lock(&xchg->mutex);
    int isEnough = -1;
    if(trader->inventory >= quantity){
        isEnough = 1;
    }else{
        isEnough = 0;
    }

    if(isEnough){
        trader_decrease_inventory(trader, quantity);

        xchg->count = xchg->count + 1;
        trader->count = trader->count + 1;
        if(price > xchg->highest_ask){
            xchg->highest_ask = price;
        }
        ORDERID * new_orderid = malloc(sizeof(ORDERID));
        memset(new_orderid, 0 , sizeof(ORDERID));

        new_orderid->orderid = xchg->orderid->orderid + 1;
        new_orderid->ask = price;
        new_orderid->quantity = quantity;
        new_orderid->trader = trader;

        ORDERID * head = xchg->orderid;

        xchg->orderid = new_orderid;
        new_orderid->next = head;
        head->prev = new_orderid;
        BRS_PACKET_HEADER * header = malloc(sizeof(BRS_PACKET_HEADER));
        memset(header, 0 , sizeof(BRS_PACKET_HEADER));
        BRS_NOTIFY_INFO * status = malloc(sizeof(BRS_NOTIFY_INFO));
        memset(status, 0 , sizeof(BRS_NOTIFY_INFO));
        status->quantity = htonl(quantity);
        status->buyer = htonl(0);
        status->price = htonl(price);
        status->seller = htonl(new_orderid->orderid);

        header->size = sizeof(BRS_NOTIFY_INFO);
        header->type = BRS_POSTED_PKT;

        trader_broadcast_packet(header,status);
        free(header);
        free(status);
        V(&semophore);

        pthread_mutex_unlock(&xchg->mutex);
        return new_orderid->orderid;

    }else{

        trader_send_nack(trader);
        pthread_mutex_unlock(&xchg->mutex);
        return 0;
    }
}

/*
 *
 * This function attempts to cancel a pending order on the exchange.
If the order is present, and was created by the same trader as is requesting cancellation,
then the order is removed from the exchange and any encumbered funds or inventory are
restored to the trader's account.  In this case, a CANCELED packet containing the parameters
of the canceled order is broadcast to all logged-in traders.
 *
 *
 * Attempt to cancel a pending order.
 * If successful, the quantity of the canceled order is returned in a variable,
 * and a CANCELED packet containing details of the canceled order is
 * broadcast to all logged-in traders.
 *
 * @param xchg  The exchange from which the order is to be cancelled.
 * @param trader  The trader cancelling the order is to be posted,
 * which must be the same as the trader who originally posted the order.
 * @param id  The order ID of the order to be cancelled.
 * @param quantity  Pointer to a variable in which to return the quantity
 * of the order that was canceled.  Note that this need not be the same as
 * the original order amount, as the order could have been partially
 * fulfilled by trades.
 * @return  0 if the order was successfully cancelled, -1 otherwise.
 * Note that cancellation might fail if a trade fulfills and removes the
 * order before this function attempts to cancel it.
 */
int exchange_cancel(EXCHANGE *xchg, TRADER *trader, orderid_t order,
                    quantity_t *quantity){
    pthread_mutex_lock(&xchg->mutex);
    ORDERID* cursor = xchg->orderid;
    while(cursor != NULL){
        if(cursor->orderid == order){
            ORDERID * prev = cursor->prev;
            ORDERID * next = cursor->next;
            if(prev == NULL && next != NULL){
                xchg->orderid = next;
            }else if(prev != NULL && next == NULL){
                prev->next = NULL;
            }else if(prev != NULL && next != NULL){
                prev->next = next;
                next->prev = prev;
            }


            if(cursor->bid > 0){
                ORDERID * cursor1 = cursor->next;
                xchg->highest_bid = 0;
                while(cursor1 != NULL){
                    if(cursor1->bid > xchg->highest_bid){
                        xchg->highest_bid = cursor1->bid;
                    }
                    cursor1 = cursor1->next;
                }
                xchg->count = xchg-> count - 1;
                trader->count = trader->count -1;
                trader_increase_balance(trader,(cursor->bid) * (cursor->quantity));
                BRS_NOTIFY_INFO * status = malloc(sizeof(BRS_NOTIFY_INFO));
                memset(status, 0 , sizeof(BRS_NOTIFY_INFO));
                status->quantity = htonl(cursor->quantity);
                status->buyer = htonl(cursor->orderid);
                status->price = htonl(cursor->bid);
                status->seller = htonl(0);

                BRS_PACKET_HEADER * header = malloc(sizeof(BRS_PACKET_HEADER));
                memset(header, 0 , sizeof(BRS_PACKET_HEADER));
                header->type = BRS_CANCELED_PKT;
                header->size = sizeof(BRS_NOTIFY_INFO);
                trader_broadcast_packet(header,status);
                free(header);
                free(status);
            }
            else if(cursor->ask>0){
                xchg->highest_ask = 0;
                ORDERID * cursor1 = cursor->next;
                while(cursor1 != NULL){
                    if(cursor1->ask > xchg->highest_ask){
                        xchg->highest_ask = cursor1->ask;
                    }
                    cursor1 = cursor1->next;
                }
                xchg->count = xchg-> count - 1;
                trader->count = trader->count - 1;
                trader_increase_inventory(trader,cursor->quantity);
                BRS_NOTIFY_INFO * status = malloc(sizeof(BRS_NOTIFY_INFO));
                memset(status, 0 , sizeof(BRS_NOTIFY_INFO));
                status->quantity = htonl(cursor->quantity);
                status->buyer = htonl(0);
                status->price = htonl(cursor->bid);
                status->seller = htonl(cursor->orderid);

                BRS_PACKET_HEADER * header = malloc(sizeof(BRS_PACKET_HEADER));
                memset(header, 0 , sizeof(BRS_PACKET_HEADER));
                header->type = BRS_CANCELED_PKT;
                header->size = sizeof(BRS_NOTIFY_INFO);
                trader_broadcast_packet(header,status);
                free(header);
                free(status);
            }
            pthread_mutex_unlock(&xchg->mutex);
            free(cursor);
            return 0;
        }
        cursor = cursor->next;
    }
    pthread_mutex_unlock(&xchg->mutex);
    return -1;
}