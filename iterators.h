//
// Created by victo on 10/04/2018.
//

#ifndef CINQ_ITERATORS_H
#define CINQ_ITERATORS_H

#include "iterator.h"
#include <stddef.h>

//Distinct, Except, GroupBy, GroupJoin, Intersect, Join, OrderBy, SelectMany, ContainsDefault, SelectMany, SequenceEqual
struct Iterable_t *it_new(const void *data, size_t item_size, size_t count);

#define IT_NEW(data) it_new(data, sizeof(typeof(*(data))), sizeof(data) / sizeof(typeof(*(data))))

struct Iterable_t *it_where(struct Iterable_t *, function_predicate);

struct Iterable_t *it_select(struct Iterable_t *, function_selector);

struct Iterable_t *it_take(struct Iterable_t *, size_t);

struct Iterable_t *it_takeWhile(struct Iterable_t *, function_predicate);

struct Iterable_t *it_skip(struct Iterable_t *, size_t);

struct Iterable_t *it_skipWhile(struct Iterable_t *, function_predicate);

struct Iterable_t *it_append(struct Iterable_t *, const void *);

struct Iterable_t *it_prepend(struct Iterable_t *, const void *);

struct Iterable_t *it_defaultIfEmpty(struct Iterable_t *, const void *);

struct Iterable_t *it_empty();

struct Iterable_t *it_range(long long int, long long int);

struct Iterable_t *it_repeat(const void *, size_t);

struct Iterable_t *it_concat(struct Iterable_t *, struct Iterable_t *);

struct Iterable_t *it_reverse(struct Iterable_t *);

size_t it_count(struct Iterable_t *);

bool it_all(struct Iterable_t *, function_predicate);

bool it_any(struct Iterable_t *, function_predicate);

bool it_hasnext(struct Iterator_t *);

bool it_contains(struct Iterable_t *, const void *, function_comparator);

void *it_toArray(struct Iterable_t *, size_t, size_t *);

#define IT_TOARRAY(iterable, T) ((T*)it_toArray(iterable, sizeof(T)))

struct Iterator_t *it_iterator(struct Iterable_t *);

const void *it_at(struct Iterable_t *iterable, size_t index);

#define IT_AT(iterable, index, T) *((const T*)it_at(iterable, index))

const void *it_last(struct Iterable_t *);

#define IT_LAST(iterable, T) (*(const T*)it_last(iterable))

const void *it_first(struct Iterable_t *);

#define IT_FIRST(iterable, T) (*(const T*)it_first(iterable))

const void *it_next(struct Iterator_t *);

#define IT_NEXT(iterator, T) (*(const T*)it_next(iterator))

void *it_accumulate(struct Iterable_t *, const void *, function_accumulator);

#define IT_ACCUMULATE(iterable, seed, accumulator, T) (*(T*)it_accumulate(iterator, seed, accumulator))

#define IT_ITERATE(iterable, storage, action)do{ \
struct Iterable_t* ___iterable = iterable; \
struct Iterator_t* ___iterator = ___iterable->get_iterator(___iterable); \
const storage* ___value; \
while(___iterator->has_next(___iterator)){ \
    ___value = ___iterator->next(___iterator);\
    action \
}}while(0)

#define IT_ITERATE_A(array, storage, action)do{ \
struct Iterable_t* ___iterable = IT_NEW(array); \
struct Iterator_t* ___iterator = ___iterable->get_iterator(___iterable); \
const storage* ___value; \
while(___iterator->has_next(___iterator)){ \
    ___value = ___iterator->next(___iterator);\
action \
}}while(0)

#define EMPTY_STATEMENT

#ifdef __GNUC__

#define IT_SUM(iterable, T)({ \
T ___sum = 0;\
IT_ITERATE(iterable, T, ___sum += *___value;);\
___sum;})

#define IT_AVERAGE(iterable, T)({ \
T ___sum = 0; size_t ___count = 0;\
IT_ITERATE(iterable, T, ___sum += *___value; ___count++;);\
if(count != 0) ___sum /= ___count; \
___sum;})

#define IT_MAX(iterable, T)({\
T ___max = 0; bool ___hasElements = false;\
IT_ITERATE(iterable, T, ___max = !_hasElements || ___max < *___value? *___value : ___max; hasElements = true;);\
___max;})

#define IT_MIN(iterable, T)({\
T ___min = 0; bool ___hasElements = false;\
IT_ITERATE(iterable, T, ___min = !_hasElements || ___min > *___value? *___value : ___min; hasElements = true;);\
___min;})

#endif //__GNUC__
#endif //CINQ_ITERATORS_H
