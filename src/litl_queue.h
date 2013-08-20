/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_QUEUE_H_
#define LITL_QUEUE_H_

// data structure for holding the trace file names in the same order as they
//    arrived
struct litl_queue_node {
    char* filename;
    struct litl_queue_node* next;
};

typedef struct litl_queue_t{
    struct litl_queue_node *head;
    struct litl_queue_node *tail;
} litl_queue_t;

void litl_init_queue(litl_queue_t* queue);

void litl_enqueue(litl_queue_t* queue, char* val);

char* litl_dequeue(litl_queue_t* queue);

void litl_print(litl_queue_t* queue);

void litl_delqueue(litl_queue_t* queue);

#endif /* LITL_QUEUE_H_ */
