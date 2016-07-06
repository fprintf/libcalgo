#include "htable.h"

#include <stdlib.h>

/** 
 * SYNOPSIS
 * htable.c - defines a simple separately chained auto-expanding hashtable structure with many useful methods 
 **/

struct htable {
    htable_hfunc_t hfunc; /* function used to hash the keys */
    size_t entries;      /* number of entries currently in hash, used for calculatinog load factor */
    size_t size;         /* size of the buckets array */
    htable_bucket_t ** buckets; /* array of pointers to htable_bucket_t type for easy logic checking */
    htable_bucket_t * list; /* elements referenced in a double-linked list for ease of traversal/deletion */
};

/*****************************************************************************
 * TYPE CONSTRUCTOR
 *****************************************************************************/
htable_t * new(size_t suggested_size, htable_hfunc_t hfunc) {
    htable_t * htab = malloc(sizeof *htab);

    if (htab) {
        *htab = (htable_t){
            .size = suggested_size,
            .hfunc = hfunc
        };

        htab->buckets = calloc(htab->size, sizeof *htab->buckets);
        /* Be sure to clean up if we failed to allocate memory */
        if (!htab->buckets) {
            free(htab);
            htab = NULL;
        }
    }

    return htab;
}

/*****************************************************************************
 * PRIVATE METHODS
 *****************************************************************************/
static htable_bucket_t * find(htable_t * htab, void * key, size_t len, htable_bucket_t *** ref) {
    size_t hash = htab->hfunc(key, len);
    htable_bucket_t * value;

    if ( (value = htab->buckets[hash % htab->size]) ) {
        if (ref) *ref = &htab->buckets[hash % htab->size];

        do {
            if (value->hash == hash) 
                break;

            /* Update our reference */
            if (ref) *ref = &value->next;

            value = value->next;

        } while (value);
    }

    return value;
}

/* Allocate and initialize a bucket object */
static htable_bucket_t * make_bucket(htable_t * htab, void * key, size_t len, void * data) {
    htable_bucket_t * bucket = malloc(sizeof *bucket);
    
    if (bucket) {
        *bucket = (htable_bucket_t){ .key = key, .data = data };
        bucket->hash = htab->hfunc(bucket->key, len);
    }

    return bucket;
}

/* Insert an existing/initialized bucket object into the hash table */
static void insert_bucket(htable_t * htab, htable_bucket_t * bucket) {
    size_t slot = bucket->hash % htab->size;

    /* Starting at htab->buckets[slot] find a NULL
     * pointer and assing our new bucket there
     */
    htable_bucket_t ** search = &htab->buckets[slot];
    while (*search)
        search = &((*search)->next);

    *search = bucket;
    ++htab->entries;

    /* Now insert the bucket into the double linked list, the order doesn't matter */
    /* 0 = prev, 1 = next */
    bucket->link[1] = htab->list;
    htab->list = bucket;
    /* Fix previous link now if there was a bucket there already */
    if (bucket->link[1])
        bucket->link[1]->link[0] = bucket;
}

/* Calculate the load factor as a double */
static inline double load_factor(htable_t * htab) {
    return htab->entries / htab->size;
}

/* Resize the table if load_factor is too high */
static void resize_table(htable_t * htab) {
    if (load_factor(htab) < HTAB_LOAD_MAX) 
        return;

    htable_t * newtab = new(htab->size * 2, htab->hfunc);
    if (newtab) {
        /* Go through and add every item into our new hash */
        htable_bucket_t * bucket = htab->list;
        while (bucket) {
            insert_bucket(newtab, bucket);
            bucket = bucket->link[1];
        }

        /* Cleanup memory from old table now */
        free(htab->buckets);
        /* Now copy everything back into the orignal hash */
        *htab = *newtab;
        /* Cleanup housing we used from new table */
        free(newtab);
    }
}


/*****************************************************************************
 * PUBLIC METHODS
 *****************************************************************************/

void * lookup(htable_t * htab, void * key, size_t len) {
    htable_bucket_t * value = find(htab, key, len, NULL);
    void * data = NULL;

    if (value)
        data = value->data;

    return data;
}

void * delete(htable_t * htab, void * key, size_t len) {
    htable_bucket_t ** ref = NULL;
    void * data = NULL;

    htable_bucket_t * bucket = find(htab, key, len, &ref);
    if (bucket) {
        data = bucket->data;
        /* Remove the entry from the buckets table */
        *ref = bucket->next;

        /* Remove the entry from bucket list */
        (*ref)->link[1] = bucket->link[1];
        if (bucket->link[1]) {
            bucket->link[1]->link[0] = *ref;
        }

        /* Keep track that we have less entries now */
        --htab->entries;
    }

    return data;
}

void * store(htable_t * htab, void * key, size_t len, void * data) {
    resize_table(htab);

    htable_bucket_t * bucket = make_bucket(htab, key, len, data);
    if (bucket) {
        insert_bucket(htab, bucket);
    }
    
    return data;
}

/* Run a function on each element */
/* The function should return a boolean value, on true it will
 * continue, on false it will stop the loop prematurely */
void foreach(htable_t * htab, int (*each)(void * key, void * data)) {
    htable_bucket_t * p = htab->list;

    while (p) {
        if (!each(p->key, p->data))
            break;
        p = p->link[1];
    }
}

/* Run a function on each key */
void foreach_keys(htable_t * htab, int (*each)(void * key)) {
    htable_bucket_t * p = htab->list;

    while (p) {
        if (!each(p->key)) 
            break;
        p = p->link[1];
    }
}

/* Run a function on each piece of data */
void foreach_values(htable_t * htab, int (*each)(void * data)) {
    htable_bucket_t * p = htab->list;

    while (p) {
        if (!each(p->data))
            break;
        p = p->link[1];
    }
}

/* Returns the internal bucket list for more advanced traversal methods
 * Must be traversed like a linked list 
 *
 * Example:
 * htable_bucket_t * list = list(htable);
 * while (list) {
 *     // do something with list object 
 *     list = list->link[1];
 * }
 *
 * NOTE: This is mainly used to allow for composition using this as a
 * base, but you must be VERY careful using this as you're accessing
 * a changing state item here. So you must store the state yourself
 * in your own structure, and say when a delete() or a store()
 * happens on the table you must update your reference to this variable also
 * as it would have gone stale.
 */
htable_bucket_t * list(htable_t * htab) {
    return htab->list;
}

/*
 * Accessors, just used for refernce capabilities
 */
size_t size(htable_t * htab) { return htab->size; }
size_t entries(htable_t * htab) { return htab->entries; }
htable_hfunc_t hfunc(htable_t * htab) { return htab->hfunc; }


/*****************************************************************************
 * TYPE INTERFACE METHODS DYNAMIC LINKAGE
 *****************************************************************************/
const struct htable_interface hash = {
    .new = new,
    .store = store,
    .lookup = lookup,
    .delete = delete,
    .foreach = foreach,
    .foreach_keys = foreach_keys,
    .foreach_values = foreach_values,
    .list = list,
    .size = size,
    .entries = entries,
    .hfunc = hfunc,
};

