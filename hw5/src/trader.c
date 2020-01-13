//
// Created by student on 12/7/19.
//

#include "trader.h"
#include "debug.h"
#include "csapp.h"
#include "trader_struct.h"
#include <inttypes.h>

/*
 * Initialize the trader module.
 * This must be called before doing calling any other functions in this
 * module.
 *
 * @return 0 if initialization succeeds, -1 otherwise.
 */
int trader_init(void){

    for(int i = 0; i< MAX_TRADERS; i++){
        trade[i].balance = 0;
        trade[i].inventory = 0;
        trade[i].quantity = 0;
        trade[i].fd = -1;
        trade[i].name = NULL;
        trade[i].count = 0;
        pthread_mutex_init(&trade[i].mutex,NULL);
    }
    debug("trader_init");

    return 0;
}

/*
 * Finalize the trader module, freeing all associated resources.
 * This should be called when the traders module is no longer required.
 */
void trader_fini(void){
    for(int i = 0; i< MAX_TRADERS; i++){
        trade[i].balance = 0;
        trade[i].inventory = 0;
        trade[i].quantity = 0;
        trade[i].fd = -1;
        if (trade[i].name != NULL) free(trade[i].name);
        trade[i].count = 0;
        pthread_mutex_destroy(&trade[i].mutex);
    }

    debug("trader finished");

}

/*
 * Attempt to log in a trader with a specified user name.
 *
 * @param clientfd  The file descriptor of the connection to the client.
 * @param name  The trader's user name, which is copied by this function.
 * @return A pointer to a TRADER object, in case of success, otherwise NULL.
 *
 * The login can fail if the specified user name is already logged in.
 * If the login succeeds then a mapping is recorded from the specified avatar
 * to a TRADER object that is created for this client and returned as the
 * result of the call.  The returned TRADER object has a reference count equal
 * to one.  This reference should be "owned" by the thread that is servicing
 * this client, and it should not be released until the client has logged out.
 */
TRADER *trader_login(int fd, char *name){
    for(int i = 0; i< MAX_TRADERS; i++){
        if(trade[i].name == NULL){
            continue;
        }
        if(fd == trade[i].fd && strcmp(name,trade[i].name) == 0){
            return &(trade[i]);
        }
    }


    for(int i = 0 ; i< MAX_TRADERS; i++){
        if(trade[i].name == NULL){
            trade[i].name = malloc(strlen(name)+1);
            memset(trade[i].name, 0, strlen(name)+1);
            strcpy(trade[i].name,name);
            trade[i].fd = fd;
            trade[i].quantity = 0;
            trade[i].inventory = 0;
            trade[i].balance = 0;
            trade[i].count = 0;
            info("%d", i);
            return &trade[i];
        }
    }
    debug("name is %s, fd is %d", name, fd);
    return NULL;
}

/*
 * Log out a trader.
 *
 * @param trader  The trader to be logged out.
 *
 * All outstanding orders for the specified trader are cancelled, and the calling
 * thread blocks until there are no orders for this trader left in the exchange.
 * Then the specified trader is removed from the traders map.
 * This function "consumes" one reference to the TRADER object by calling
 * trader_unref().  This will have the effect of causing the TRADER object
 * to be freed as soon as any references to it currently held by other threads
 * have been released.
 */
void trader_logout(TRADER *trader){

    trader_unref(trader,"logout" );
    trader->fd = -1;
    debug("trader log out");

}

/*
 * Increase the reference count on a trader by one.
 *
 * @param trader  The TRADER whose reference count is to be increased.
 * @param why  A string describing the reason why the reference count is
 * being increased.  This is used for debugging printout, to help trace
 * the reference counting.
 * @return  The same TRADER object that was passed as a parameter.
 */
TRADER *trader_ref(TRADER *trader, char *why){
    pthread_mutex_lock(&trader->mutex);
    debug("reference trader count number");
    trader->count = trader->count + 1;
    pthread_mutex_unlock(&trader->mutex);

    return trader;
}

/*
 * Decrease the reference count on a trader by one.
 *
 * @param trader  The TRADER whose reference count is to be decreased.
 * @param why  A string describing the reason why the reference count is
 * being increased.  This is used for debugging printout, to help trace
 * the reference counting.
 *
 * If after decrementing, the reference count has reached zero, then the
 * trader and its contents are freed.
 */
void trader_unref(TRADER *trader, char *why){

    pthread_mutex_lock(&trader->mutex);
    trader->count = trader->count -1;
    debug("unreference trader count number");

    if(trader->count == 0){
        for(int i = 0; i< MAX_TRADERS; i++){
            trade[i].balance = 0;
            trade[i].inventory = 0;
            trade[i].quantity = 0;
            trade[i].fd = -1;
            if (trade[i].name != NULL) free(trade[i].name);
            trade[i].count = 0;
            pthread_mutex_destroy(&trade[i].mutex);
        }
        pthread_mutex_unlock(&trader->mutex);


    }else{
        pthread_mutex_unlock(&trader->mutex);
    }

}

/*
 * Send a packet to the client for a trader.
 *
 * @param trader  The TRADER object for the client who should receive
 * the packet.
 * @param pkt  The packet to be sent.
 * @param data  Data payload to be sent, or NULL if none.
 * @return 0 if transmission succeeds, -1 otherwise.
 *
 * Once a client has connected and successfully logged in, this function
 * should be used to send packets to the client, as opposed to the lower-level
 * proto_send_packet() function.  The reason for this is that the present
 * function will obtain exclusive access to the trader before calling
 * proto_send_packet().  The fact that exclusive access is obtained before
 * sending means that multiple threads can safely call this function to send
 * to the client, and these calls will be properly serialized.
 */
int trader_send_packet(TRADER *trader, BRS_PACKET_HEADER *pkt, void *data){
    pthread_mutex_lock(&trader->mutex);
    debug("trader send package");
    proto_send_packet(trader->fd, pkt,  data);
    pthread_mutex_unlock(&trader->mutex);
    return 0;
}

/*
 * Broadcast a packet to all currently logged-in traders.
 *
 * @param pkt  The packet to be sent.
 * @param data  Data payload to be sent, or NULL if none.
 * @return 0 if broadcast succeeds, -1 otherwise.
 */
int trader_broadcast_packet(BRS_PACKET_HEADER *pkt, void *data){
    for(int i = 0; i< MAX_TRADERS; i++){
        if(trade[i].fd != -1) {
            proto_send_packet(trade[i].fd, pkt, data);
        }
    }
    debug("trader finish broadcast");

    return 0;
}

/*
 * Send an ACK packet to the client for a trader.
 *
 * @param trader  The TRADER object for the client who should receive
 * the packet.
 * @param infop  Pointer to the optional data payload for this packet,
 * or NULL if there is to be no payload.
 * @return 0 if transmission succeeds, -1 otherwise.
 */
int trader_send_ack(TRADER *trader, BRS_STATUS_INFO *info){
    BRS_PACKET_HEADER * ack = malloc(sizeof(BRS_PACKET_HEADER));
    memset(ack,0, sizeof(BRS_PACKET_HEADER));
    ack->type = BRS_ACK_PKT;
    ack->size = sizeof(BRS_STATUS_INFO);
    debug("quantity is %" PRIu32 "\n",info->quantity);
    int ret = proto_send_packet(trader->fd, ack,  info);
    free(ack);
    debug("trader send acknowledge");

    if(ret == 0) debug("send ack success");
    else debug("send ack fail");
    return 0;
}

/*
 * Send an NACK packet to the client for a trader.
 *
 * @param trader  The TRADER object for the client who should receive
 * the packet.
 * @return 0 if transmission succeeds, -1 otherwise.
 */
int trader_send_nack(TRADER *trader){
    BRS_PACKET_HEADER * nack = malloc(sizeof(BRS_PACKET_HEADER));
    memset(nack,0, sizeof(BRS_PACKET_HEADER));
    nack->type = BRS_NACK_PKT;
    nack->size = 0;
    int ret = proto_send_packet(trader->fd, nack,  NULL);
    free(nack);
    debug("trader send non acknowledge");

    if(ret == 0) debug("send nack success");
    else debug("send nack fail");

    return 0;
}

/*
 * Increase the balance for a trader.
 *
 * @param trader  The trader whose balance is to be increased.
 * @param amount  The amount by which the balance is to be increased.
 */
void trader_increase_balance(TRADER *trader, funds_t amount){
    pthread_mutex_lock(&trader->mutex);
    trader->balance = trader->balance + amount;
    debug("trader increase balance amount");
    pthread_mutex_unlock(&trader->mutex);

    return;
}

/*
 * Attempt to decrease the balance for a trader.
 *
 * @param trader  The trader whose balance is to be decreased.
 * @param amount  The amount by which the balance is to be decreased.
 * @return 0 if the original balance is at least as great as the
 * amount of decrease, -1 otherwise.
 */
int trader_decrease_balance(TRADER *trader, funds_t amount){
    if(trader->balance < amount) return -1;
    pthread_mutex_lock(&trader->mutex);
    trader->balance = trader->balance - amount;
    debug("trader decrease balance amount");

    pthread_mutex_unlock(&trader->mutex);


    return 0;
}

/*
 * Increase the inventory for a trader by a specified quantity.
 *
 * @param trader  The trader whose inventory is to be increased.
 * @param amount  The amount by which the inventory is to be increased.
 */
void trader_increase_inventory(TRADER *trader, quantity_t quantity){
    pthread_mutex_lock(&trader->mutex);
    debug("fffff %" PRIu32 "\n", quantity);

    debug("increased quantity before is %" PRIu32 "\n",trader->quantity);

    trader->inventory += quantity;
    debug("increased quantity to %" PRIu32 "\n",trader->quantity);
    debug("trader increase inventory quantity");
    pthread_mutex_unlock(&trader->mutex);


    return;
}

/*
 * Attempt to decrease the inventory for a trader by a specified quantity.
 *
 * @param trader  The trader whose inventory is to be decreased.
 * @param amount  The amount by which the inventory is to be decreased.
 * @return 0 if the original inventory is at least as great as the
 * amount of decrease, -1 otherwise.
 */
int trader_decrease_inventory(TRADER *trader, quantity_t quantity){
    if(trader->inventory <quantity) return -1;
    pthread_mutex_lock(&trader->mutex);
    trader->inventory = trader->inventory - quantity;
    debug("trader decrease inventory quantity");
    pthread_mutex_unlock(&trader->mutex);
    return 0;
}