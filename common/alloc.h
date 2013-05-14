#ifndef ALLOC_H
#define ALLOC_H

typedef struct {
    void *(*malloc)(int);
    void *(*realloc)(void*,int);
    void (*free)(void*);
} alloc_t;

extern const alloc_t *gpkg_allocator;

#define gpkg_malloc gpkg_allocator->malloc
#define gpkg_realloc gpkg_allocator->realloc
#define gpkg_free gpkg_allocator->free

#endif