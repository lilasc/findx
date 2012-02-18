#ifndef STACK_H
#define STACK_H

#include <stdio.h>

#define STACK_SIZE 512

struct stack {
    char value[STACK_SIZE];
    int index;
};

int push(struct stack* s, char v);
char pop(struct stack* s);
char top(struct stack* s);
void reverse(struct stack* s);
void clear(struct stack* s);

#endif
