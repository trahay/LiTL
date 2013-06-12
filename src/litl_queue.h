/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_QUEUE_H_
#define LITL_QUEUE_H_

// data structure for holding the trace file names in the same order as they arrived
struct node {
    char* filename;
    struct node* next;
};

typedef struct {
    struct node *head;
    struct node *tail;
} litl_queue;

void init_queue();

void enqueue(char* val);

char* dequeue();

void print();

void delqueue();

#endif /* LITL_QUEUE_H_ */
