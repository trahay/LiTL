/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "litl_queue.h"

void litl_init_queue(litl_queue* queue) {
    queue->head = NULL;
    queue->tail = NULL;
}

void litl_enqueue(litl_queue* queue, char* val) {
    int length;
    length = strlen(val) + 1;

    if (queue->tail == NULL ) {
        queue->tail = (struct node *) malloc(sizeof(struct node));
        queue->tail->filename = (char *) malloc(length);
        strcpy(queue->tail->filename, val);
        queue->tail->next = NULL;
        queue->head = queue->tail;
    } else {
        struct node *tmp;
        tmp = (struct node *) malloc(sizeof(struct node));
        tmp->filename = (char *) malloc(length);
        strcpy(tmp->filename, val);
        tmp->next = NULL;
        queue->tail->next = tmp;
        queue->tail = tmp;
    }
}

char* litl_dequeue(litl_queue* queue) {
    if (queue->head == NULL )
        return NULL ;

    char* val;
    val = queue->head->filename;
    queue->head = queue->head->next;

    return val;
}

void litl_print(litl_queue *queue) {
    while (queue->head != NULL ) {
        printf("%s \n", queue->head->filename);
        queue->head = queue->head->next;
    }

}

void litl_delqueue(litl_queue* queue) {
    struct node *tmp;

    while (queue->head != NULL ) {
        tmp = queue->head;
        queue->head = queue->head->next;
        free(tmp->filename);
        free(tmp);
    }
}
