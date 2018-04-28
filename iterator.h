//
// Created by victo on 10/04/2018.
//

#ifndef CINQ_ITERATOR_H
#define CINQ_ITERATOR_H

#include <stdbool.h>

struct Iterator_t;
struct Iterable_t;

typedef bool (*function_hasnext)(struct Iterator_t *);

typedef const void *(*function_next)(struct Iterator_t *);

typedef const void *(*function_selector)(const void *);

typedef bool (*function_predicate)(const void *);

typedef void *(*function_copy)(const void *);

typedef void *(*function_accumulator)(const void *, const void *);

typedef struct Iterator_t *(*function_get_iterator)(struct Iterable_t *);

typedef int (*function_comparator)(const void *, const void *);

struct Iterator_t {
    void *extra;
    function_next next;
    function_hasnext has_next;
};

struct Iterable_t {
    void *extra;
    function_get_iterator get_iterator;
};

struct Iterator_t *Iterator_new(void *extra, function_next next, function_hasnext has_next);

struct Iterable_t *Iterable_new(void *extra, function_get_iterator get_iterator);

#endif //CINQ_ITERATOR_H
