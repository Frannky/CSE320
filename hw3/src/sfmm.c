/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sfmm.h>
#include "debug.h"
#include "sfmm.h"

#define FOURWORDS 8
#define MIN_BLOCK_SIZE 32

sf_prologue* prologue;
sf_epilogue* epilogue;
struct sf_block* insert_node(size_t size);
sf_block* coalesce(sf_block* node);
void initial_list();
int get_list(size_t size);
sf_block* get_block(int index, size_t allocated_size);

void initial_list(){
    for(int i = 0; i< NUM_FREE_LISTS; i++){
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
        sf_free_list_heads[i].header = 0;
    }
}

sf_block* get_block(int index, size_t allocated_size){
    for(int i = index; i < NUM_FREE_LISTS; i++){
        sf_block *temp = &sf_free_list_heads[i];
//        sf_show_heap();
        if(temp->body.links.next == &sf_free_list_heads[index] && temp->body.links.prev == &sf_free_list_heads[index]) continue;
        sf_block *ptr = temp->body.links.next;
        while(ptr != temp){
            if((ptr->header & BLOCK_SIZE_MASK) >= allocated_size){
                return ptr;
            }
            ptr = ptr->body.links.next;
        }
    }

    return NULL;
}

int get_list(size_t size){

    if(size <= MIN_BLOCK_SIZE)
        return 0;
    else if(size > MIN_BLOCK_SIZE && size <= 2*MIN_BLOCK_SIZE)
        return 1;
    else if(size > 2*MIN_BLOCK_SIZE && size <= 4*MIN_BLOCK_SIZE)
        return 2;
    else if(size > 4*MIN_BLOCK_SIZE && size <= 8*MIN_BLOCK_SIZE)
        return 3;
    else if(size > 8*MIN_BLOCK_SIZE && size <= 16*MIN_BLOCK_SIZE)
        return 4;
    else if(size > 16*MIN_BLOCK_SIZE && size <= 32*MIN_BLOCK_SIZE)
        return 5;
    else if(size > 32*MIN_BLOCK_SIZE && size <= 64*MIN_BLOCK_SIZE)
        return 6;
    else if(size > 64*MIN_BLOCK_SIZE && size <= 128*MIN_BLOCK_SIZE)
        return 7;
    else if(size > 128*MIN_BLOCK_SIZE)//size > 128*MIN_BLOCK_SIZE
        return 8;
    else
        return -1;
}

struct sf_block* insert_node(size_t size){
    if(size <= MIN_BLOCK_SIZE)
        return &sf_free_list_heads[0];
    else if(size > MIN_BLOCK_SIZE && size <= 2*MIN_BLOCK_SIZE)
        return &sf_free_list_heads[1];
    else if(size > 2*MIN_BLOCK_SIZE && size <= 4*MIN_BLOCK_SIZE)
        return &sf_free_list_heads[2];
    else if(size > 4*MIN_BLOCK_SIZE && size <= 8*MIN_BLOCK_SIZE)
        return &sf_free_list_heads[3];
    else if(size > 8*MIN_BLOCK_SIZE && size <= 16*MIN_BLOCK_SIZE)
        return &sf_free_list_heads[4];
    else if(size > 16*MIN_BLOCK_SIZE && size <= 32*MIN_BLOCK_SIZE)
        return &sf_free_list_heads[5];
    else if(size > 32*MIN_BLOCK_SIZE && size <= 64*MIN_BLOCK_SIZE)
        return &sf_free_list_heads[6];
    else if(size > 64*MIN_BLOCK_SIZE && size <= 128*MIN_BLOCK_SIZE)
        return &sf_free_list_heads[7];
    else if(size > 128*MIN_BLOCK_SIZE)//size > 128*MIN_BLOCK_SIZE
        return &sf_free_list_heads[8];
    else
        return NULL;
}

sf_block* coalesce(sf_block* node){
    int last = 0;
    int first = 0;
    size_t size = node->header & BLOCK_SIZE_MASK;

    if((void *) node + size + FOURWORDS == (void *) epilogue) last =1;
    else last = 0;

    if((void *) node - MIN_BLOCK_SIZE == (void *) prologue) first =1;
    else first = 0;

    if(first){
        sf_block *next_block =  (void *)node + size;
        if(((next_block->header) & (THIS_BLOCK_ALLOCATED)) == THIS_BLOCK_ALLOCATED) return node;
        else{
            size_t next_block_size = next_block->header & BLOCK_SIZE_MASK;
            size_t total_size = size + next_block_size;

            // removw node and pre_block from free_list

            //remove node from_list
            sf_block* pre_block1 = node->body.links.prev;
            sf_block* nex_block1 = node->body.links.next;
            pre_block1->body.links.next = nex_block1;
            nex_block1->body.links.prev = pre_block1;

            //remove next_block from_list
            sf_block* pre_block2 = next_block->body.links.prev;
            sf_block* nex_block2 = next_block->body.links.next;
            pre_block2->body.links.next = nex_block2;
            nex_block2->body.links.prev = pre_block2;

//            memset(node,0 , size);
//            memset(next_block,0,next_block_size);

            node ->header = total_size | PREV_BLOCK_ALLOCATED;
            sf_block * p = (void *)node + total_size;
            p->prev_footer = (node->header) ^ sf_magic();

            //add pre_block into free_list

            struct sf_block* insert_list_node = insert_node(total_size);
            sf_block* first = insert_list_node->body.links.next;
            sf_block* last = insert_list_node->body.links.prev;
            if(first == insert_list_node && last == insert_list_node){
                insert_list_node->body.links.next = node;
                insert_list_node->body.links.prev = node;
                node->body.links.prev = insert_list_node;
                node->body.links.next = insert_list_node;
            }else{
                insert_list_node->body.links.next = node;
                node->body.links.next = first;
                first->body.links.prev = node;
                node->body.links.prev = insert_list_node;
                insert_list_node->body.links.prev = last;
                last->body.links.next = insert_list_node;
            }

            return node;
        }
    }

    else if(last){
        if(((node->header) & PREV_BLOCK_ALLOCATED) == PREV_BLOCK_ALLOCATED) return node;
        else{
            size_t pre_size = ((node->prev_footer) ^ sf_magic()) & BLOCK_SIZE_MASK;
            sf_block * pre_block = (void *) node - pre_size;
            size_t pre = (pre_block->header) & PREV_BLOCK_ALLOCATED;
            size_t total_size = pre_size + size;

            // removw node and pre_block from free_list

            //remove node from_list
            sf_block* pre_block1 = node->body.links.prev;
            sf_block* nex_block1 = node->body.links.next;
            pre_block1->body.links.next = nex_block1;
            nex_block1->body.links.prev = pre_block1;

            //remove pre_block from_list
            sf_block* pre_block2 = pre_block->body.links.prev;
            sf_block* nex_block2 = pre_block->body.links.next;
            pre_block2->body.links.next = nex_block2;
            nex_block2->body.links.prev = pre_block2;

//            memset(node,0 , size);
//            memset(pre_block,0,pre_size);

            pre_block ->header = total_size | pre;
            sf_block * p = (void *)pre_block + total_size;
            p->prev_footer = (pre_block->header) ^ sf_magic();
            epilogue->header = 0 | THIS_BLOCK_ALLOCATED;
            p->header = epilogue->header;
            //add pre_block into free_list

            struct sf_block* insert_list_node = insert_node(total_size);
            sf_block* first = insert_list_node->body.links.next;
            sf_block* last = insert_list_node->body.links.prev;
            if(first == insert_list_node && last == insert_list_node){
                insert_list_node->body.links.next = pre_block;
                insert_list_node->body.links.prev = pre_block;
                pre_block->body.links.prev = insert_list_node;
                pre_block->body.links.next = insert_list_node;
            }else{
                insert_list_node->body.links.next = pre_block;
                pre_block->body.links.next = first;
                first->body.links.prev = pre_block;
                pre_block->body.links.prev = insert_list_node;
                insert_list_node->body.links.prev = last;
                last->body.links.next = insert_list_node;
            }

            return pre_block;
        }
    }

    else{

        size_t pre = (node->header) & PREV_BLOCK_ALLOCATED;
        size_t this_block_size = (node->header) & BLOCK_SIZE_MASK;
        sf_block * next_block = (void *) node + this_block_size;
        size_t next = (next_block->header) & THIS_BLOCK_ALLOCATED;
        size_t pre_block_size = ((node->prev_footer) ^ sf_magic()) & BLOCK_SIZE_MASK;
        size_t next_block_size = (next_block->header) & BLOCK_SIZE_MASK;

        if(pre == PREV_BLOCK_ALLOCATED && next == THIS_BLOCK_ALLOCATED){
            return  node;
        }
        else if(pre == 0 && next == THIS_BLOCK_ALLOCATED){
            size_t total_size = this_block_size + pre_block_size;
            sf_block * pre_block = (void *)node - pre_block_size;

            // removw node and pre_block from free_list

            //remove node from_list
            sf_block* pre_block1 = node->body.links.prev;
            sf_block* nex_block1 = node->body.links.next;
            pre_block1->body.links.next = nex_block1;
            nex_block1->body.links.prev = pre_block1;

            //remove pre_block from_list
            sf_block* pre_block2 = pre_block->body.links.prev;
            sf_block* nex_block2 = pre_block->body.links.next;
            pre_block2->body.links.next = nex_block2;
            nex_block2->body.links.prev = pre_block2;

//            memset(node,0 , size);
//            memset(pre_block,0,pre_block_size);
            size_t pre1 = (pre_block->header) & PREV_BLOCK_ALLOCATED;
            pre_block ->header = total_size | pre1;
            sf_block * p = (void *)pre_block + total_size;
            p->prev_footer = (pre_block->header) ^ sf_magic();
            p->header = (p->header & BLOCK_SIZE_MASK) | THIS_BLOCK_ALLOCATED;
            size_t p_size = p->header & BLOCK_SIZE_MASK;
            sf_block *q = (void *)p + p_size;
            q->prev_footer = p->header ^ sf_magic();
            //add pre_block into free_list

            struct sf_block* insert_list_node = insert_node(total_size);
            sf_block* first = insert_list_node->body.links.next;
            sf_block* last = insert_list_node->body.links.prev;
            if(first == insert_list_node && last == insert_list_node){
                insert_list_node->body.links.next = pre_block;
                insert_list_node->body.links.prev = pre_block;
                pre_block->body.links.prev = insert_list_node;
                pre_block->body.links.next = insert_list_node;
            }else{
                insert_list_node->body.links.next = pre_block;
                pre_block->body.links.next = first;
                first->body.links.prev = pre_block;
                pre_block->body.links.prev = insert_list_node;
                insert_list_node->body.links.prev = last;
                last->body.links.next = insert_list_node;
            }

            return pre_block;
        }

        else if(pre == PREV_BLOCK_ALLOCATED && next == 0){

            size_t total_size = this_block_size + next_block_size;

            // removw node and pre_block from free_list

            //remove node from_list
            sf_block* pre_block1 = node->body.links.prev;
            sf_block* nex_block1 = node->body.links.next;
            pre_block1->body.links.next = nex_block1;
            nex_block1->body.links.prev = pre_block1;
//            sf_show_heap();
            //remove next_block from_list
            sf_block* pre_block2 = next_block->body.links.prev;
            sf_block* nex_block2 = next_block->body.links.next;
            pre_block2->body.links.next = nex_block2;
            nex_block2->body.links.prev = pre_block2;

//            memset(node,0 , size);
//            memset(next_block,0,next_block_size);
//            sf_block * pre_block = (void*)node - pre_block_size;

//            node->prev_footer = pre_block->header ^ sf_magic();
            node ->header = total_size | pre;
            sf_block * p = (void *)node + total_size;
            p->prev_footer = (node->header) ^ sf_magic();
            //add pre_block into free_list

            struct sf_block* insert_list_node = insert_node(total_size);
            sf_block* first = insert_list_node->body.links.next;
            sf_block* last = insert_list_node->body.links.prev;
            if(first == insert_list_node && last == insert_list_node){
                insert_list_node->body.links.next = node;
                insert_list_node->body.links.prev = node;
                node->body.links.prev = insert_list_node;
                node->body.links.next = insert_list_node;
            }else{
                insert_list_node->body.links.next = node;
                node->body.links.next = first;
                first->body.links.prev = node;
                node->body.links.prev = insert_list_node;
                insert_list_node->body.links.prev = last;
                last->body.links.next = insert_list_node;
            }
            return node;
        }
        else if(pre ==0 && next == 0){
            size_t total_size = this_block_size + pre_block_size + next_block_size;

            sf_block * pre_block = (void *)node - pre_block_size;
            // remove node and pre_block from free_list

            //remove node from_list
            sf_block* pre_block1 = node->body.links.prev;
            sf_block* nex_block1 = node->body.links.next;
            pre_block1->body.links.next = nex_block1;
            nex_block1->body.links.prev = pre_block1;

            //remove next_block from_list
            sf_block* pre_block2 = next_block->body.links.prev;
            sf_block* nex_block2 = next_block->body.links.next;
            pre_block2->body.links.next = nex_block2;
            nex_block2->body.links.prev = pre_block2;
//            sf_show_heap();
            //remove pre_block from_list
            sf_block* pre_block3 = pre_block->body.links.prev;
            sf_block* nex_block3 = pre_block->body.links.next;
            pre_block3->body.links.next = nex_block3;
            nex_block3->body.links.prev = pre_block3;

//            memset(pre_block,0, pre_block_size);
//            memset(node,0 , this_block_size);
//            memset(next_block,0,next_block_size);
            size_t pre1 = (pre_block->header) & PREV_BLOCK_ALLOCATED;
            pre_block ->header = total_size | pre1;
            sf_block * p = (void *)pre_block + total_size;
            p->prev_footer = (pre_block->header) ^ sf_magic();
            //add pre_block into free_list

            struct sf_block* insert_list_node = insert_node(total_size);
            sf_block* first = insert_list_node->body.links.next;
            sf_block* last = insert_list_node->body.links.prev;
            if(first == insert_list_node && last == insert_list_node){
                insert_list_node->body.links.next = pre_block;
                insert_list_node->body.links.prev = pre_block;
                pre_block->body.links.prev = insert_list_node;
                pre_block->body.links.next = insert_list_node;
            }else{
                insert_list_node->body.links.next = pre_block;
                pre_block->body.links.next = first;
                first->body.links.prev = pre_block;
                pre_block->body.links.prev = insert_list_node;
                insert_list_node->body.links.prev = last;
                last->body.links.next = insert_list_node;
            }

            return pre_block;
        }
    }
    return NULL;
}


void *sf_malloc(size_t size) {

    //size equal to 0 ,no need to allocate memory
    if(size == 0) return NULL;

    //start with initialization for prologue and epilogue
    if(sf_mem_start() == sf_mem_end()){
        void* initial = sf_mem_grow();
        if(initial == NULL){
            sf_errno = ENOMEM;
            return NULL;
        }
        initial_list();
        //initializing prologue
        prologue = (sf_prologue *)initial;
//        memset(initial,0,PAGE_SZ);

        //how to initialize padding1????
        prologue->padding1 = 0; // this is previous footer
        prologue->header = (MIN_BLOCK_SIZE) | PREV_BLOCK_ALLOCATED | THIS_BLOCK_ALLOCATED;

        //Footer contents must always be identical to header contents XOR'ed with sf_magic().
        prologue->footer = (prologue->header) ^ sf_magic();

        prologue->unused1 = NULL;
        prologue->unused2 = NULL;

        //initializing epilogue
        epilogue = (sf_epilogue *) (sf_mem_end() - FOURWORDS);
//        memset(sf_mem_end() - 2*FOURWORDS,0,2*FOURWORDS);


        epilogue->header = 0 | THIS_BLOCK_ALLOCATED;

        //allocate size for the first time
        size_t allocated_size = size + 2*FOURWORDS;

        // make allocated size to be multiple of 16
        if(allocated_size <= MIN_BLOCK_SIZE) allocated_size = MIN_BLOCK_SIZE;
        else{
            if(allocated_size % (2*FOURWORDS) != 0) allocated_size = ((allocated_size >> 4) + 1) << 4;
            else allocated_size = allocated_size;
        }


        //remain block size
        size_t avaliable_size = PAGE_SZ - MIN_BLOCK_SIZE - 2*FOURWORDS;
        // why after 16336 call sf_mem_grow will return NULL
        //allocated size is 16352 > 16336  how can it not be a NULL
        //continuously growth on the memory until meets memory requirement
        while(allocated_size > avaliable_size){
            if(sf_mem_grow() == NULL){
                sf_errno = ENOMEM;
                sf_block* next_free_block = sf_mem_start() + MIN_BLOCK_SIZE;
                next_free_block->header = avaliable_size | PREV_BLOCK_ALLOCATED;
                sf_block * p = (void *)next_free_block + avaliable_size;
                p->prev_footer = next_free_block->header ^ sf_magic();
                p->header = epilogue->header;
                struct sf_block* insert_list_node = insert_node(avaliable_size);
                sf_block* first = insert_list_node->body.links.next;
                sf_block* last = insert_list_node->body.links.prev;
                if(first == insert_list_node && last == insert_list_node){
                    insert_list_node->body.links.next = next_free_block;
                    insert_list_node->body.links.prev = next_free_block;
                    next_free_block->body.links.prev = insert_list_node;
                    next_free_block->body.links.next = insert_list_node;
                }else{
                    insert_list_node->body.links.next = next_free_block;
                    next_free_block->body.links.next = first;
                    first->body.links.prev = next_free_block;
                    next_free_block->body.links.prev = insert_list_node;
                    insert_list_node->body.links.prev = last;
                    last->body.links.next = insert_list_node;
                }
                return NULL;
            }
//            memset(epilogue, 0, FOURWORDS);
            epilogue = (sf_epilogue *) (sf_mem_end() - FOURWORDS);
//            memset(epilogue,0,FOURWORDS);

            //what is last bit of header of epilogue????????
            epilogue->header = 0 | THIS_BLOCK_ALLOCATED;

            avaliable_size += PAGE_SZ;
        }

        //struct of allocated memory
        sf_block* allocated_memo = sf_mem_start() + MIN_BLOCK_SIZE;

        allocated_memo->prev_footer = (prologue->header) ^ sf_magic();
        //should an allocated block has pre_footer or its own footer
        allocated_memo->header = ((allocated_size) | PREV_BLOCK_ALLOCATED | THIS_BLOCK_ALLOCATED);
        //prologue has a footer should the block below it also has a previous footer or they are the same


        avaliable_size -= allocated_size;

        if(avaliable_size < MIN_BLOCK_SIZE){
            allocated_size += avaliable_size;
            allocated_memo->header = ((allocated_size) | PREV_BLOCK_ALLOCATED | THIS_BLOCK_ALLOCATED);
            epilogue->header = 0 | THIS_BLOCK_ALLOCATED | PREV_BLOCK_ALLOCATED;
            return sf_mem_start()+ MIN_BLOCK_SIZE + 2*FOURWORDS;
        }

        sf_block * next_free_block = sf_mem_start() + sizeof(sf_prologue) + allocated_size- FOURWORDS;
        next_free_block->header = avaliable_size | PREV_BLOCK_ALLOCATED;
        next_free_block->prev_footer = (allocated_memo->header) ^ sf_magic();

        sf_block * p = sf_mem_start() + MIN_BLOCK_SIZE + allocated_size + avaliable_size;
        p->prev_footer = (next_free_block->header) ^ sf_magic();
        epilogue->header = 0 | THIS_BLOCK_ALLOCATED;
        p->header = epilogue->header;

        struct sf_block* insert_list_node = insert_node(avaliable_size);
        sf_block* first = insert_list_node->body.links.next;
        sf_block* last = insert_list_node->body.links.prev;
        if(first == insert_list_node && last == insert_list_node){
            insert_list_node->body.links.next = next_free_block;
            insert_list_node->body.links.prev = next_free_block;
            next_free_block->body.links.prev = insert_list_node;
            next_free_block->body.links.next = insert_list_node;
        }else{
            insert_list_node->body.links.next = next_free_block;
            next_free_block->body.links.next = first;
            first->body.links.prev = next_free_block;
            next_free_block->body.links.prev = insert_list_node;
            insert_list_node->body.links.prev = last;
            last->body.links.next = insert_list_node;
        }
//        sf_show_heap();
//        sf_show_free_lists();
//        sf_show_blocks();
        coalesce(next_free_block);
        return sf_mem_start()+ MIN_BLOCK_SIZE + 2*FOURWORDS;
    }
    else{
        size_t allocated_size = size + 2*FOURWORDS;
        // make allocated size to be multiple of 16
        if(allocated_size <= MIN_BLOCK_SIZE) allocated_size = MIN_BLOCK_SIZE;
        else{
            if(allocated_size % (2*FOURWORDS) != 0) allocated_size = ((allocated_size >> 4) + 1) << 4;
            else allocated_size = allocated_size;
        }

        sf_block * allocated_block;
        int index = get_list(allocated_size);
        if(index == -1){
            allocated_block = NULL;
        }else{
            allocated_block = get_block(index,allocated_size);
        }

        if(allocated_block != NULL){
            size_t available_size = (allocated_block->header) & BLOCK_SIZE_MASK;

            //remove_from_list
            sf_block* pre_block = allocated_block->body.links.prev;
            sf_block* nex_block = allocated_block->body.links.next;
            pre_block->body.links.next = nex_block;
            nex_block->body.links.prev = pre_block;

            if(available_size - allocated_size < MIN_BLOCK_SIZE){
                allocated_size = available_size;
                int pre = (allocated_block->header) & PREV_BLOCK_ALLOCATED;
                size_t pre_size = (allocated_block->prev_footer ^ sf_magic()) & BLOCK_SIZE_MASK;
                sf_block * pre_block = (void *)allocated_block - pre_size;
                allocated_block->prev_footer = pre_block->header ^ sf_magic();
                allocated_block->header = allocated_size | THIS_BLOCK_ALLOCATED | pre;
                sf_block * p = (void *)allocated_block + allocated_size;
                p->prev_footer = (allocated_block->header) ^ sf_magic();
                p->header = (p->header & BLOCK_SIZE_MASK) | (p->header & THIS_BLOCK_ALLOCATED) | PREV_BLOCK_ALLOCATED;
                size_t p_size = p->header & BLOCK_SIZE_MASK;
                sf_block *q = (void *)p + p_size;
                q->prev_footer = p->header ^ sf_magic();
                if((void *)allocated_block + allocated_size == epilogue - FOURWORDS){
                    epilogue->header = 0 | THIS_BLOCK_ALLOCATED | PREV_BLOCK_ALLOCATED;
                    p->header = epilogue->header;
                }

                return (void *)allocated_block + 2*FOURWORDS;

            }else{
                available_size = available_size - allocated_size;
            }
            int pre = (allocated_block->header) & PREV_BLOCK_ALLOCATED;
            size_t pre_size = (allocated_block->prev_footer ^ sf_magic()) & BLOCK_SIZE_MASK;
            sf_block * pre_block1 = (void *)allocated_block - pre_size;
            allocated_block->prev_footer = pre_block1 ->header ^ sf_magic();
            allocated_block->header = allocated_size | THIS_BLOCK_ALLOCATED | pre;


            //split the block
            sf_block* next_free_block = (void *)allocated_block + allocated_size;
            next_free_block->header = available_size | PREV_BLOCK_ALLOCATED;
            next_free_block->prev_footer = (allocated_block->header) ^ sf_magic();

            sf_block * p = (void *)next_free_block + available_size;
            p->prev_footer = (next_free_block->header) ^ sf_magic();
            if((void *)next_free_block + available_size == epilogue - FOURWORDS){
                epilogue->header = 0 | THIS_BLOCK_ALLOCATED;
                p->header = epilogue->header;
            }
//            epilogue->header = 0 | THIS_BLOCK_ALLOCATED | PREV_BLOCK_ALLOCATED;

            struct sf_block* insert_list_node = insert_node(available_size);


            //add new node to the list
            sf_block* first = insert_list_node->body.links.next;
            sf_block* last = insert_list_node->body.links.prev;
            if(first == insert_list_node && last == insert_list_node){
                insert_list_node->body.links.next = next_free_block;
                insert_list_node->body.links.prev = next_free_block;
                next_free_block->body.links.prev = insert_list_node;
                next_free_block->body.links.next = insert_list_node;
            }else{
                insert_list_node->body.links.next = next_free_block;
                next_free_block->body.links.next = first;
                first->body.links.prev = next_free_block;
                next_free_block->body.links.prev = insert_list_node;
                insert_list_node->body.links.prev = last;
                last->body.links.next = insert_list_node;
            }

            coalesce(next_free_block);

            return (void *)allocated_block + 2*FOURWORDS;

        }
        else{
            size_t pre = (epilogue->header) & PREV_BLOCK_ALLOCATED;

            void* new_page = sf_mem_grow();
            if(new_page == NULL){
                sf_errno = ENOMEM;
                return NULL;
            }
            sf_block* new_block = (void *)new_page - 2*FOURWORDS;
            new_block->header = PAGE_SZ | pre;
            sf_block* last1 = (void *)new_block + PAGE_SZ;
            last1->prev_footer = new_block->header ^ sf_magic();
            size_t pre_size = (new_block->prev_footer ^ sf_magic()) & BLOCK_SIZE_MASK;
            sf_block* pre_block = (void *)new_block - pre_size;
            size_t pre1 = (new_block->prev_footer ^ sf_magic()) & THIS_BLOCK_ALLOCATED;
//            memset(new_page,0,PAGE_SZ);
            epilogue = (sf_epilogue *) (sf_mem_end() - FOURWORDS);
            struct sf_block* insert_list_node1 = insert_node(PAGE_SZ);
            sf_block* first2 = insert_list_node1->body.links.next;
            sf_block* last2 = insert_list_node1->body.links.prev;
            if(first2 == insert_list_node1 && last2 == insert_list_node1){
                insert_list_node1->body.links.next = new_block;
                insert_list_node1->body.links.prev = new_block;
                new_block->body.links.prev = insert_list_node1;
                new_block->body.links.next = insert_list_node1;
            }else{
                insert_list_node1->body.links.next = new_block;
                new_block->body.links.next = first2;
                first2->body.links.prev = new_block;
                new_block->body.links.prev = insert_list_node1;
                insert_list_node1->body.links.prev = last2;
                last2->body.links.next = insert_list_node1;
            }
            coalesce(new_block);
            size_t avaliable_size;
            if(pre1 == THIS_BLOCK_ALLOCATED){
                avaliable_size= PAGE_SZ;
            }
            else{
                avaliable_size = pre_size + PAGE_SZ;
            }
            int i = 1;

            while(allocated_size > avaliable_size){
                void * new_page1 = sf_mem_grow();
                if(new_page1 == NULL){
                    sf_block* next_free_block;
                    if(pre1 == THIS_BLOCK_ALLOCATED){
                        next_free_block = new_block;
                    }
                    else{
                        next_free_block = pre_block;
                    }

                    next_free_block->header = avaliable_size | PREV_BLOCK_ALLOCATED;
                    sf_block * p = (void *)next_free_block + avaliable_size;
                    p->prev_footer = next_free_block->header ^ sf_magic();
//                sf_show_heap();
                    if((void *)p == (void *)epilogue - FOURWORDS){
                        epilogue->header = 0 | THIS_BLOCK_ALLOCATED;
                        p->header = epilogue->header;
                    }
                    struct sf_block* insert_list_node = insert_node(avaliable_size);
                    sf_block* first = insert_list_node->body.links.next;
                    sf_block* last = insert_list_node->body.links.prev;
                    if(first == insert_list_node && last == insert_list_node){
                        insert_list_node->body.links.next = next_free_block;
                        insert_list_node->body.links.prev = next_free_block;
                        next_free_block->body.links.prev = insert_list_node;
                        next_free_block->body.links.next = insert_list_node;
                    }else{
                        insert_list_node->body.links.next = next_free_block;
                        next_free_block->body.links.next = first;
                        first->body.links.prev = next_free_block;
                        next_free_block->body.links.prev = insert_list_node;
                        insert_list_node->body.links.prev = last;
                        last->body.links.next = insert_list_node;
                    }
                    coalesce(next_free_block);
                    sf_errno = ENOMEM;
                    return NULL;
                }
                i++;
//                memset(sf_mem_start(), 0, i*PAGE_SZ);
                epilogue = (sf_epilogue *) (sf_mem_end() - FOURWORDS);
//                memset(epilogue,0,FOURWORDS);

                avaliable_size += PAGE_SZ;


            }


            if(avaliable_size - allocated_size < MIN_BLOCK_SIZE){
                allocated_size = avaliable_size;
                new_block->header = allocated_size | THIS_BLOCK_ALLOCATED | pre;
                sf_block * p = (void *)new_block + allocated_size;
                p->prev_footer = (new_block->header) ^ sf_magic();
                p->header = (p->header & BLOCK_SIZE_MASK) | (p->header & THIS_BLOCK_ALLOCATED) | PREV_BLOCK_ALLOCATED;
                size_t p_size = p->header & BLOCK_SIZE_MASK;
                sf_block *q = (void *)p + p_size;
                q->prev_footer = p->header ^ sf_magic();
                if((void *)new_block + allocated_size == (void*)epilogue - FOURWORDS){
                    epilogue->header = 0 | THIS_BLOCK_ALLOCATED | PREV_BLOCK_ALLOCATED;
                    p->header = epilogue->header;
                }else{
                    epilogue->header = 0 | THIS_BLOCK_ALLOCATED;
                }

                return (void *)new_block + 2*FOURWORDS;

            }else{
                avaliable_size = avaliable_size - allocated_size;
            }
            if(pre1 == THIS_BLOCK_ALLOCATED){
                new_block = new_block;
            }
            else{
                new_block = pre_block;
            }
            new_block->header = allocated_size | THIS_BLOCK_ALLOCATED | PREV_BLOCK_ALLOCATED;
            //remove_from_list
            sf_block* pre_block1 = new_block->body.links.prev;
            sf_block* nex_block1 = new_block->body.links.next;
            pre_block1->body.links.next = nex_block1;
            nex_block1->body.links.prev = pre_block1;
            sf_block * next_free_block = (void *) new_block + allocated_size;
            next_free_block->prev_footer = (new_block->header) ^ sf_magic();
            next_free_block->header = avaliable_size | PREV_BLOCK_ALLOCATED;
            sf_block * p = (void *)next_free_block + avaliable_size;
            p->prev_footer = next_free_block->header ^ sf_magic();
            if((void *)next_free_block + avaliable_size == (void*)epilogue - FOURWORDS){
                epilogue->header = 0 | THIS_BLOCK_ALLOCATED;
                p->header = epilogue->header;
            }
            struct sf_block* insert_list_node = insert_node(avaliable_size);


            //add new node to the list
            sf_block* first = insert_list_node->body.links.next;
            sf_block* last = insert_list_node->body.links.prev;
            if(first == insert_list_node && last == insert_list_node){
                insert_list_node->body.links.next = next_free_block;
                insert_list_node->body.links.prev = next_free_block;
                next_free_block->body.links.prev = insert_list_node;
                next_free_block->body.links.next = insert_list_node;
            }else{
                insert_list_node->body.links.next = next_free_block;
                next_free_block->body.links.next = first;
                first->body.links.prev = next_free_block;
                next_free_block->body.links.prev = insert_list_node;
                insert_list_node->body.links.prev = last;
                last->body.links.next = insert_list_node;
            }

            coalesce(next_free_block);

            return (void *)new_block + 2*FOURWORDS;
        }

    }

}

void sf_free(void *pp) {
    if(pp == NULL) abort();
    if (pp < sf_mem_start() + MIN_BLOCK_SIZE + FOURWORDS) abort();
    if (pp > sf_mem_end() - FOURWORDS) abort();

    sf_block* block = pp - 2*FOURWORDS;
    size_t pre = (block->prev_footer ^ sf_magic()) & THIS_BLOCK_ALLOCATED;
    if((block->header & THIS_BLOCK_ALLOCATED) == 0) abort();
    size_t block_size = block->header & BLOCK_SIZE_MASK;
    if(block_size < MIN_BLOCK_SIZE || block_size % (2*FOURWORDS) != 0) abort();
    size_t pre_block_size = ((block->prev_footer) ^ sf_magic()) & BLOCK_SIZE_MASK;
    if(pre_block_size < MIN_BLOCK_SIZE || pre_block_size % (2*FOURWORDS) != 0) abort();
    sf_block * p = (void *) block + block_size;
    if((void *) block < (void *)prologue + MIN_BLOCK_SIZE) abort();
    if((void *) block + block_size > (void *)epilogue - FOURWORDS) abort();
//    size_t test = block->header;
//    size_t test1 = p->prev_footer ^ sf_magic();
    if((p->prev_footer) != (block->header ^ sf_magic())) abort();
    if(((p->prev_footer ^ sf_magic()) & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED) abort();
    if(pre == 0 && (block->header & PREV_BLOCK_ALLOCATED) != 0) abort();
    if(pre == 2 && (block->header & PREV_BLOCK_ALLOCATED) != 1) abort();
    if(pre == THIS_BLOCK_ALLOCATED) block->header = block_size | PREV_BLOCK_ALLOCATED;
    else block->header = block_size;
    sf_block * pre_block = (void *)block - pre_block_size;
    block->prev_footer = pre_block->header ^ sf_magic();
    p->prev_footer = block->header ^ sf_magic();
    p->header = ((p->header) & BLOCK_SIZE_MASK) | ((p->header) & THIS_BLOCK_ALLOCATED);
    size_t p_size = p->header & BLOCK_SIZE_MASK;
    sf_block * q = (void *)p + p_size;
    q->prev_footer = p->header ^ sf_magic();
    struct sf_block* insert_list_node = insert_node(block_size);
//    sf_show_heap();

    //add new node to the list
    sf_block* first = insert_list_node->body.links.next;
    sf_block* last = insert_list_node->body.links.prev;
    if(first == insert_list_node && last == insert_list_node){
        insert_list_node->body.links.next = block;
        insert_list_node->body.links.prev = block;
        block->body.links.prev = insert_list_node;
        block->body.links.next = insert_list_node;
    }else{
        insert_list_node->body.links.next = block;
        block->body.links.next = first;
        first->body.links.prev = block;
        block->body.links.prev = insert_list_node;
        insert_list_node->body.links.prev = last;
        last->body.links.next = insert_list_node;
    }
//    sf_show_heap();
    coalesce(block);
}

void *sf_realloc(void *pp, size_t rsize) {
    if(pp == NULL) {
        sf_errno = EINVAL;
        return NULL;
    }
    if (pp < sf_mem_start() + MIN_BLOCK_SIZE + FOURWORDS) {
        sf_errno = EINVAL;
        return NULL;
    }
    if (pp > sf_mem_end() - FOURWORDS) {
        sf_errno = EINVAL;
        return NULL;
    }

    sf_block* block = pp - 2*FOURWORDS;
    size_t pre = (block->prev_footer ^ sf_magic()) & THIS_BLOCK_ALLOCATED;
    if((block->header & THIS_BLOCK_ALLOCATED) == 0) {
        sf_errno = EINVAL;
        return NULL;
    }
    size_t block_size = block->header & BLOCK_SIZE_MASK;
    if(block_size < MIN_BLOCK_SIZE || block_size % (2*FOURWORDS) != 0) {
        sf_errno = EINVAL;
        return NULL;
    }
    size_t pre_block_size = (block->prev_footer ^ sf_magic()) & BLOCK_SIZE_MASK;
    if(pre_block_size < MIN_BLOCK_SIZE || pre_block_size % (2*FOURWORDS) != 0) {
        sf_errno = EINVAL;
        return NULL;
    }
    sf_block * p = (void *) block + block_size;
    if((void *) block < (void *)prologue + MIN_BLOCK_SIZE) {
        sf_errno = EINVAL;
        return NULL;
    }
    if((void *) block + block_size > (void *)epilogue - FOURWORDS) {
        sf_errno = EINVAL;
        return NULL;
    }
    // why????
    if(pre == 0 && (block->header & PREV_BLOCK_ALLOCATED) != 0) {
        sf_errno = EINVAL;
        return NULL;
    }
    if(pre == 2 && (block->header & PREV_BLOCK_ALLOCATED) != 1) {
        sf_errno = EINVAL;
        return NULL;
    }
    if((p->prev_footer) != (block->header ^ sf_magic())) {
        sf_errno = EINVAL;
        return NULL;
    }
    if(((p->prev_footer ^ sf_magic()) & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED) {
        sf_errno = EINVAL;
        return NULL;
    }


    if(rsize == 0){
        sf_free(pp);
        return NULL;
    }

    size_t new_block_size = rsize;
    new_block_size = new_block_size + 2*FOURWORDS;

    if(new_block_size < MIN_BLOCK_SIZE) new_block_size = MIN_BLOCK_SIZE;
    else{
        if(new_block_size % (2*FOURWORDS) != 0){
            new_block_size = ((new_block_size >> 4) + 1) <<4;
        }
    }

    if(new_block_size > block_size){
        void* ptr = sf_malloc(new_block_size - 2*FOURWORDS);
        if(ptr == NULL) {
            sf_errno = ENOMEM;
            return NULL;
        }
        memcpy(ptr,pp,block_size);
        sf_free(pp);
        return ptr;
    }
    else if(new_block_size < block_size){
        if(block_size - new_block_size < MIN_BLOCK_SIZE){
            block->header = block_size | (block->header & THIS_BLOCK_ALLOCATED) | (block->header & PREV_BLOCK_ALLOCATED);
            sf_block *p1 = (void *) block + block_size;
            p1->prev_footer  = block->header ^ sf_magic();
            p1->header = (p1->header & BLOCK_SIZE_MASK) | (p1->header & THIS_BLOCK_ALLOCATED) | PREV_BLOCK_ALLOCATED;
            sf_block * q = (void *)p1 + (p1->header & BLOCK_SIZE_MASK);
            q->prev_footer = p1->header ^ sf_magic();
            return pp;
        }
        else{

            block->header = new_block_size | THIS_BLOCK_ALLOCATED | (block->header & PREV_BLOCK_ALLOCATED);
            sf_block *next_free_block = (void *) block + new_block_size;
            next_free_block->prev_footer = (block->header) ^ sf_magic();
            //block_size = 80, new_block_size= 32 pre_block_size =32
            next_free_block->header = (block_size - new_block_size) | PREV_BLOCK_ALLOCATED;

            sf_block *p = (void *)next_free_block + (block_size - new_block_size);
            p->prev_footer = next_free_block->header ^ sf_magic();
            p->header = (p->header & BLOCK_SIZE_MASK) | (p->header & THIS_BLOCK_ALLOCATED);
            size_t p_size = p->header & BLOCK_SIZE_MASK;
            sf_block *q = (void *)p + p_size;
            q->prev_footer = p->header ^ sf_magic();
            struct sf_block* insert_list_node = insert_node(block_size);
            //add new node to the list
            sf_block* first = insert_list_node->body.links.next;
            sf_block* last = insert_list_node->body.links.prev;
            if(first == insert_list_node && last == insert_list_node){
                insert_list_node->body.links.next = next_free_block;
                insert_list_node->body.links.prev = next_free_block;
                next_free_block->body.links.prev = insert_list_node;
                next_free_block->body.links.next = insert_list_node;
            }else{
                insert_list_node->body.links.next = next_free_block;
                next_free_block->body.links.next = first;
                first->body.links.prev = next_free_block;
                next_free_block->body.links.prev = insert_list_node;
                insert_list_node->body.links.prev = last;
                last->body.links.next = insert_list_node;
            }

            coalesce(next_free_block);
            return pp;
        }
    }
    else{
        return pp;
    }
}
