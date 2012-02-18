#include "stack.h"

int push(struct stack* s, char v)
{
    if (s->index == STACK_SIZE - 1)
        return -1;

    s->value[++s->index] = v;

    return s->index;
}

char pop(struct stack* s)
{
    if (s->index < 0)
        return -1;

    return s->value[s->index--];
}

char top(struct stack* s)
{
    if (s->index < 0)
        return -1;

    return s->value[s->index];
}

void reverse(struct stack* s)
{
    int i;
    char tmp;

    for (i = 0; i < (s->index + 1) / 2; i++)
    {
        tmp = s->value[i];
        s->value[i] = s->value[s->index - i];
        s->value[s->index - i] = tmp;
    }
}

void clear(struct stack* s)
{
    s->index = -1;
}
