#include "htable.h"

#include <stdio.h>

int print_entry(void * key, void * data) {
    char * msg = key;
    int * number = data;

    printf("got key: [%s] value: [%d]\n", msg, *number);
}

size_t myhashfunc(void * key, size_t len) {
    unsigned char * byte = key;
    unsigned char * end = byte + len;
    size_t hash = 5831;

    while (byte < end) {
        hash = (hash << 1) + *byte++;
    }

    printf("Hash %zu\n", hash);
    return hash;
}

int main(void) {
    htable_t * htab = hash.new(16, myhashfunc);

    hash.store(htab, "some random shit", sizeof "some random shit", &(int){5});
    hash.foreach(htab, print_entry);
    hash.store(htab, "random shit 3", sizeof "random shit 3", &(int){3});
    hash.foreach(htab, print_entry);
    hash.store(htab, "another shit", sizeof "another shit", &(int){6});
    hash.foreach(htab, print_entry);
}

