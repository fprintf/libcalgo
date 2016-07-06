#ifndef HTABLE_GUARD__H__
#define HTABLE_GUARD__H__

/* When load_factor is equal to or above this, resize the hash */
#define HTAB_LOAD_MAX 1.0

#include <stddef.h>

/* Htable hash function signature */
typedef size_t (*htable_hfunc_t)(void * key, size_t len);


/* Htable buckets, best not to toy with this lightly */
typedef struct bucket {
    size_t hash;
    void * key;
    void * data;
    struct bucket * next;
    struct bucket * link[2]; /* 0 - prev, 1 - next, traversal linked list access used for foreach */
} htable_bucket_t;

/* Hash table opaque type */
typedef struct htable htable_t;

/* Interface */
extern const struct htable_interface hash;

struct htable_interface {
    /* Construction functions */
    htable_t * (*new_jenkins)(void);    /* bob jenkins hash function (uses reasonable starting size) */
    htable_t * (*new_multi)(void);      /* multiplicative hash function (uses reasonable starting size) */
    htable_t * (*new_best)(void);       /* use the best hash, just an alias to new_jenkins currently */
    htable_t * (*new)(size_t, htable_hfunc_t); 
    void (*free)(htable_t * table, int (*)(void * key, void * data)); /* destroys the hash table and frees all buckets */
    /* Basic usage */
    void * (*store)(htable_t *, void * key, size_t len, void * data);
    void * (*lookup)(htable_t *, void * key, size_t len);
    void * (*delete)(htable_t *, void * key, size_t len);
    /* iteration */
    void (*foreach)(htable_t *, int (*)(void * key, void * data));
    void (*foreach_keys)(htable_t *, int (*)(void * key));
    void (*foreach_values)(htable_t *, int (*)(void * data));
    /* Accessors */
    htable_bucket_t * (*list)(htable_t *);
    size_t (*size)(htable_t *);
    size_t (*entries)(htable_t *);
    htable_hfunc_t (*hfunc)(htable_t *);
};

#endif
