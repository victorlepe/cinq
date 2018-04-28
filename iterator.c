//
// Created by victo on 27/04/2018.
//

#include <malloc.h>
#include "iterator.h"

struct Iterator_t *Iterator_new(void *extra, function_next next, function_hasnext has_next) {
    struct Iterator_t *iterator = malloc(sizeof(struct Iterator_t));
    iterator->extra = extra;
    iterator->has_next = has_next;
    iterator->next = next;
    return iterator;
}

struct Iterable_t *Iterable_new(void *extra, function_get_iterator get_iterator) {
    struct Iterable_t *iterable = malloc(sizeof(struct Iterable_t));
    iterable->extra = extra;
    iterable->get_iterator = get_iterator;
    return iterable;
}