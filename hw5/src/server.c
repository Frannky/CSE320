//
// Created by student on 12/3/19.
//

#include "server.h"
#include "debug.h"
#include "csapp.h"
#include "trader_struct.h"
#include "exchange_struct.h"
#include <inttypes.h>

CLIENT_REGISTRY *client_registry;

void *brs_client_service(void *arg){

    int fd =* (int*)arg;
    debug("client_service: fd number is %d", fd);
    free(arg);
    Pthread_detach(pthread_self());

    creg_register(client_registry,fd);

    TRADER * trader = NULL;

    BRS_PACKET_HEADER * hdr = malloc(sizeof(BRS_PACKET_HEADER));

    memset(hdr, 0, sizeof(BRS_PACKET_HEADER));

    void* ptr = NULL;

    while(proto_recv_packet(fd, hdr, &ptr) == 0){
        debug("enter");
        if (hdr->type == BRS_LOGIN_PKT){
            char* p = malloc(hdr->size+1);
            memcpy(p, ptr,hdr->size);
            free(ptr);
            *(p+ hdr->size) = '\0';
            trader = trader_login(fd,p);

            debug("receive login package: fd number is %d", fd);

            if(trader != NULL){

                BRS_PACKET_HEADER * acknowledge = malloc(sizeof(BRS_PACKET_HEADER));
                memset(acknowledge, 0, sizeof(BRS_PACKET_HEADER));
                acknowledge->type = BRS_ACK_PKT;
                acknowledge->size = 0;
                debug("receive login package with trader: fd number is %d", fd);

                proto_send_packet(fd, acknowledge, NULL);
                free(acknowledge);
            }else{

                BRS_PACKET_HEADER * nacknowledge = malloc(sizeof(BRS_PACKET_HEADER));
                memset(nacknowledge, 0, sizeof(BRS_PACKET_HEADER));
                nacknowledge->type = BRS_NACK_PKT;
                nacknowledge->size = 0;
                debug("receive login package with none trader: fd number is %d", fd);

                proto_send_packet(fd, nacknowledge, NULL);
                free(nacknowledge);
            }

        }else if (hdr->type == BRS_STATUS_PKT){

            debug("receive status package: fd number is %d", fd);

            BRS_STATUS_INFO * status = malloc(sizeof(BRS_STATUS_INFO));
            memset(status, 0 , sizeof(BRS_STATUS_INFO));
            status->balance = htonl(trader->balance);
            status->inventory = htonl(trader->inventory);
            status->quantity = htonl(trader->quantity);
            BRS_STATUS_INFO *infop = malloc(sizeof(BRS_STATUS_INFO));
            memset(infop, 0 , sizeof(BRS_STATUS_INFO));
            exchange_get_status(exchange,infop);
            status->last = infop->last;
            status->orderid = infop->orderid;
            status->ask = infop->ask;
            status->bid = infop->bid;
            //need to use exchange later
            trader_send_ack(trader, status);

//            free(status);

        }else if (hdr->type == BRS_DEPOSIT_PKT){

            debug("receive deposite package: fd number is %d", fd);
            BRS_FUNDS_INFO * pptr = ptr;
            trader_increase_balance(trader, ntohl(pptr->amount));

            BRS_STATUS_INFO * status = malloc(sizeof(BRS_STATUS_INFO));
            memset(status, 0 , sizeof(BRS_STATUS_INFO));
            status->balance = htonl(trader->balance);
            status->inventory = htonl(trader->inventory);
            status->quantity = htonl(trader->quantity);
            BRS_STATUS_INFO *infop = malloc(sizeof(BRS_STATUS_INFO));
            memset(infop, 0 , sizeof(BRS_STATUS_INFO));
            exchange_get_status(exchange,infop);
            status->last = infop->last;
            status->orderid = infop->orderid;
            status->ask = infop->ask;
            status->bid = infop->bid;

            trader_send_ack(trader,status);

            free(status);

        }else if (hdr->type == BRS_WITHDRAW_PKT){

            debug("receive withdraw package: fd number is %d", fd);
            BRS_FUNDS_INFO * pptr = ptr;
            int ret = trader_decrease_balance(trader, ntohl(pptr->amount));

            BRS_STATUS_INFO * status = malloc(sizeof(BRS_STATUS_INFO));
            memset(status, 0 , sizeof(BRS_STATUS_INFO));
            status->balance = htonl(trader->balance);
            status->inventory = htonl(trader->inventory);
            status->quantity = htonl(trader->quantity);
            BRS_STATUS_INFO *infop = malloc(sizeof(BRS_STATUS_INFO));
            memset(infop, 0 , sizeof(BRS_STATUS_INFO));
            exchange_get_status(exchange,infop);
            status->last = infop->last;
            status->orderid = infop->orderid;
            status->ask = infop->ask;
            status->bid = infop->bid;

            if(ret == 0){
                trader_send_ack(trader,status);
            }else{
                trader_send_nack(trader);
            }

            free(status);


        }else if (hdr->type == BRS_ESCROW_PKT){

            debug("receive escrow package: fd number is %d",fd);
            BRS_ESCROW_INFO * pptr = ptr;
            debug("quantity is %" PRIu32 "\n",ntohl(pptr->quantity));
            trader_increase_inventory(trader, ntohl(pptr->quantity));

            BRS_STATUS_INFO * status = malloc(sizeof(BRS_STATUS_INFO));
            memset(status, 0 , sizeof(BRS_STATUS_INFO));
            status->balance = htonl(trader->balance);
            status->inventory = htonl(trader->inventory);
            status->quantity = htonl(trader->quantity);
            BRS_STATUS_INFO *infop = malloc(sizeof(BRS_STATUS_INFO));
            memset(infop, 0 , sizeof(BRS_STATUS_INFO));
            exchange_get_status(exchange,infop);
            status->last = infop->last;
            status->orderid = infop->orderid;
            status->ask = infop->ask;
            status->bid = infop->bid;

            trader_send_ack(trader,status);

            free(status);
        }else if (hdr->type == BRS_RELEASE_PKT){

            debug("receive release package: fd number is %d", fd);
            BRS_ESCROW_INFO * pptr = ptr;
            int ret = trader_decrease_inventory(trader, ntohl(pptr->quantity));

            BRS_STATUS_INFO * status = malloc(sizeof(BRS_STATUS_INFO));
            memset(status, 0 , sizeof(BRS_STATUS_INFO));
            status->balance = htonl(trader->balance);
            status->inventory = htonl(trader->inventory);
            status->quantity = htonl(trader->quantity);
            BRS_STATUS_INFO *infop = malloc(sizeof(BRS_STATUS_INFO));
            memset(infop, 0 , sizeof(BRS_STATUS_INFO));
            exchange_get_status(exchange,infop);
            status->last = infop->last;
            status->orderid = infop->orderid;
            status->ask = infop->ask;
            status->bid = infop->bid;

            if(ret == 0){
                trader_send_ack(trader,status);
            }
            else{
                trader_send_nack(trader);
            }

            free(status);
        }//need to use exchange later
        else if (hdr->type == BRS_BUY_PKT){

            debug("receive buy package: fd number is %d", fd);
            BRS_ORDER_INFO * pptr = ptr;
            orderid_t id = exchange_post_buy(exchange,trader, ntohl(pptr->quantity),ntohl(pptr->price));
            BRS_STATUS_INFO * status = malloc(sizeof(BRS_STATUS_INFO));
            memset(status, 0 , sizeof(BRS_STATUS_INFO));
            status->balance = htonl(trader->balance);
            status->inventory = htonl(trader->inventory);
            status->quantity = htonl(trader->quantity);
            BRS_STATUS_INFO *infop = malloc(sizeof(BRS_STATUS_INFO));
            memset(infop, 0 , sizeof(BRS_STATUS_INFO));
            exchange_get_status(exchange,infop);
            status->last = infop->last;
            status->orderid = htonl(id);
            status->ask = infop->ask;
            status->bid = infop->bid;

            if(id > 0){
                trader_send_ack(trader,status);
            }
            else{
                trader_send_nack(trader);
            }

            free(status);
        }//need to use exchange later
        else if (hdr->type == BRS_SELL_PKT){

            debug("receive sell package: fd number is %d", fd);
            BRS_ORDER_INFO * pptr = ptr;
            orderid_t id = exchange_post_sell(exchange,trader, ntohl(pptr->quantity),ntohl(pptr->price));
            BRS_STATUS_INFO * status = malloc(sizeof(BRS_STATUS_INFO));
            memset(status, 0 , sizeof(BRS_STATUS_INFO));
            status->balance = htonl(trader->balance);
            status->inventory = htonl(trader->inventory);
            status->quantity = htonl(trader->quantity);
            BRS_STATUS_INFO *infop = malloc(sizeof(BRS_STATUS_INFO));
            memset(infop, 0 , sizeof(BRS_STATUS_INFO));
            exchange_get_status(exchange,infop);
            status->last = infop->last;
            status->orderid = htonl(id);
            status->ask = infop->ask;
            status->bid = infop->bid;

            if(id > 0){
                trader_send_ack(trader,status);
            }
            else{
                trader_send_nack(trader);
            }

            free(status);
        }//need to use exchange later
        else if (hdr->type == BRS_CANCEL_PKT){

            debug("receive sell package: fd number is %d", fd);
            BRS_CANCEL_INFO * pptr = ptr;
            quantity_t *quantity = malloc(sizeof(quantity_t));
            memset(quantity, 0, sizeof(quantity_t));
            int ret = exchange_cancel(exchange,trader, ntohl(pptr->order),quantity);

            BRS_STATUS_INFO * status = malloc(sizeof(BRS_STATUS_INFO));
            memset(status, 0 , sizeof(BRS_STATUS_INFO));
            status->balance = htonl(trader->balance);
            status->inventory = htonl(trader->inventory);
            status->quantity = htonl(*quantity);
            BRS_STATUS_INFO *infop = malloc(sizeof(BRS_STATUS_INFO));
            memset(infop, 0 , sizeof(BRS_STATUS_INFO));
            exchange_get_status(exchange,infop);
            status->last = infop->last;
            status->orderid = pptr->order;
            status->ask = infop->ask;
            status->bid = infop->bid;

            if(ret == 0){
                trader_send_ack(trader,status);
            }
            else{
                trader_send_nack(trader);
            }

            free(status);
        }
    }
    free(hdr);
    return NULL;
}