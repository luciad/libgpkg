#ifndef ALLOC_H
#define ALLOC_H

typedef struct {
    void *(*malloc)(int);
    void *(*realloc)(void*,int);
    void (*free)(void*);
} allocator_t;

#endif