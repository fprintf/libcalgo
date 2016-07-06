#include <stdlib.h>
#include <assert.h>

#include "darray.h"

#define DARRAY_DEFAULT_SIZE 16

struct darray {
    void * entries;
    size_t size;
    size_t capacity;
};

/************************************************************************
 * Methods (mapped through darray_interface export
 ************************************************************************/

/************************************************************************
 * Methods (mapped through darray_interface export
 ************************************************************************/
darray_t * new_copy(const darray_t * restrict copy) {
    darray_t * da = malloc(sizeof *da);

    assert(da && copy);

    /* Copy size, capacity and pointer (soon to be overwritten) */
    *da = *copy;
    da->entries = calloc(da->capacity, sizeof *da->entries);

    assert(da->entries);

    memcpy(da->entries, copy->entries, copy->size);
    return da;
}

darray_t * new(void) {
    darray_t * da = new_copy(&(darray_t){
        .capacity = DARRAY_DEFAULT_SIZE,
    });

    return da;
}

char make_char(const char ch) {
    return ch;
}

darray_t * new_range(const char start, const char end) {
    darray_t * da = new();

    while (start <= end) {
        char ch = make_char(start++);
        push(da, &ch);
    }

    return da;
}

void free(darray_t * da) {
    if (da) {
        free(da->entries);
        free(da);
    }
}

const char * get_error(darray_t * da);

void * pop(darray_t * da);
void * push(darray_t * da, void * data);
void * shift(darray_t * da);
void * unshift(darray_t * da, void * data);

void * get(darray_t * da, size_t index);
void * set(darray_t * da, size_t index, void * data);
void * choice(darray_t * da); // pick an entry at random
int shuffle(darray_t * da); // single-pass randomize the array

void foreach(darray_t * da, darray_foreachcb_t func);
void foreach_reverse(darray_t * da, darray_foreachcb_t func);

// Passes 'func' elements 2-by-2 cumulating the return value of the funciton in b
void * reduce(darray_t * da, darray_reducecb_t func);
void sort(darray_t * da, darray_sortcb_t func); // sorts array based on return value of 'func'
// Creates a new darray based on the return value of running 'func' on each element of 'da'
darray_t * map(darray_t * da, darray_complexcb_t func);

/************************************************************************
 * Export: darray interface mappings
 ************************************************************************/
struct darray_interface darray = {
    .new = new,
    .new_range = new_range,
    .new_copy = new_copy,

    .strerror = get_error,

    .pop = pop,
    .push = push,
    .shift = shift,
    .unshift = unshift,
    .get = get,
    .set = set,
    .choice = choice,

    .shuffle = shuffle,

    .foreach = foreach,
    .foreach_reverse = foreach_reverse,

    .reduce = reduce,
    .sort = sort,
    .map = map,
};
