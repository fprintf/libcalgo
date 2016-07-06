#ifndef DARRAY_HEADER__H__
#define DARRAY_HEADER__H__

typedef struct darray darray_t;
typedef int (*darray_foreachcb_t)(void * data);
typedef void * (*darray_reducecb_t)(void * a, void * b);
typedef void * (*darray_complexcb_t)(void * data);
typedef int (*darray_sortcb_t)(void * a, void * b);

extern struct darray_interface darray;

struct darray_interface {
    darray_t * (*new)(void);
    darray_t * (*new_range)(size_t start, size_t end);
    darray_t * (*new_copy)(darray_t * copy);
    void (*free)(darray_t * da);

    const char * (*strerror)(darray_t * da);

    void * (*pop)(darray_t * da);
    void * (*push)(darray_t * da, void * data);
    void * (*shift)(darray_t * da);
    void * (*unshift)(darray_t * da, void * data);

    void * (*get)(darray_t * da, size_t index);
    void * (*set)(darray_t * da, size_t index, void * data);
    void * (*choice)(darray_t * da); // pick an entry at random
    int (*shuffle)(darray_t * da); // single-pass randomize the array

    void (*foreach)(darray_t * da, darray_foreachcb_t func);
    void (*foreach_reverse)(darray_t * da, darray_foreachcb_t func);

    // Passes 'func' elements 2-by-2 cumulating the return value of the funciton in b
    void * (*reduce)(darray_t * da, darray_reducecb_t func);
    void (*sort)(darray_t * da, darray_sortcb_t func); // sorts array based on return value of 'func'
    // Creates a new darray based on the return value of running 'func' on each element of 'da'
    darray_t * (*map)(darray_t * da, darray_complexcb_t func);
};


#endif
