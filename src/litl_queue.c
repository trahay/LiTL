/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "litl_queue.h"

static litl_queue traces;

void init_queue() {
    traces.head = NULL;
    traces.tail = NULL;
}

void enqueue(char* val) {
    int length;
    length = strlen(val) + 1;

    if (traces.tail == NULL ) {
        traces.tail = (struct node *) malloc(sizeof(struct node));
        traces.tail->filename = (char *) malloc(length);
        strcpy(traces.tail->filename, val);
        traces.tail->next = NULL;
        traces.head = traces.tail;
    } else {
        struct node *tmp;
        tmp = (struct node *) malloc(sizeof(struct node));
        tmp->filename = (char *) malloc(length);
        strcpy(tmp->filename, val);
        tmp->next = NULL;
        traces.tail->next = tmp;
        traces.tail = tmp;
    }
}

char* dequeue() {
    if (traces.head == NULL )
        return NULL ;

    char* val;
    val = traces.head->filename;
    traces.head = traces.head->next;

    return val;
}

void print() {
    while (traces.head != NULL ) {
        printf("%s \n", traces.head->filename);
        traces.head = traces.head->next;
    }

}

void delqueue() {
    struct node *tmp;

    while (traces.head != NULL ) {
        tmp = traces.head;
        traces.head = traces.head->next;
        free(tmp->filename);
        free(tmp);
    }
}
