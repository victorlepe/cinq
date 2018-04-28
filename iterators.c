//
// Created by victo on 10/04/2018.
//

#include <malloc.h>
#include <stdint.h>
#include <mem.h>
#include <stdio.h>
#include "iterators.h"
#include "iteratordef.h"

////////////////////////////////////////////////////
// Array
////////////////////////////////////////////////////
typedef struct {
    const void *array;
    size_t item_size;
    size_t count;
} Iterable_Array;

typedef struct {
    const char *array;
    size_t item_size;
    size_t count;
    size_t current_index;
} Iterable_Array_i;

const void *array_next(Iterator *wrap) {
    Iterable_Array_i *iterator = wrap->extra;
    const char *asChar = iterator->array;
    const void *item = asChar + (iterator->item_size * iterator->current_index);
    iterator->current_index++;
    return item;
}

bool array_hasnext(Iterator *wrap) {
    Iterable_Array_i *iterator = wrap->extra;
    return iterator->current_index < iterator->count;
}

Iterator *array_iterator(Iterable *wrap) {
    Iterable_Array *iterable = wrap->extra;
    Iterable_Array_i *iterator = malloc(sizeof(Iterable_Array_i));
    iterator->count = iterable->count;
    iterator->item_size = iterable->item_size;
    iterator->array = iterable->array;
    iterator->current_index = 0;
    return Iterator_new(iterator, array_next, array_hasnext);
}

////////////////////////////////////////////////////
// Select
////////////////////////////////////////////////////

typedef struct {
    Iterable *parent;
    function_selector selector;
} Iterable_Select;

typedef struct {
    Iterator *parent;
    function_selector selector;
} Iterable_Select_i;

const void *select_next(Iterator *wrap) {
    printf("Select NExt\n");
    Iterable_Select_i *iterator = wrap->extra;
    Iterator *parent = iterator->parent;
    return iterator->selector(parent->next(parent));
}

bool select_hasnext(Iterator *wrap) {
    printf("Select HasNext\n");
    Iterable_Select_i *iterator = wrap->extra;
    Iterator *parent = iterator->parent;
    return parent->has_next(parent);
}

Iterator *select_iterator(Iterable *wrap) {
    Iterable_Select *iterable = wrap->extra;
    Iterable_Select_i *iterator = malloc(sizeof(Iterable_Select_i));
    iterator->parent = iterable->parent->get_iterator(iterable->parent);
    iterator->selector = iterable->selector;
    return Iterator_new(iterator, select_next, select_hasnext);
}

////////////////////////////////////////////////////
// Where
////////////////////////////////////////////////////

typedef struct {
    Iterable *parent;
    function_predicate predicate;
} Iterable_Where;

typedef struct {
    Iterator *parent;
    function_predicate predicate;
    bool is_fed;
    const void *fed;
} Iterable_Where_i;

void where_feed(Iterable_Where_i *iterator) {
    if(iterator->is_fed)
        return;
    Iterator *parent = iterator->parent;
    while(parent->has_next(parent) && !iterator->is_fed) {
        const void *item = parent->next(parent);
        if(iterator->predicate(item)) {
            iterator->is_fed = true;
            iterator->fed = item;
        }
    }
}

const void *where_next(Iterator *wrap) {
    Iterable_Where_i *iterator = wrap->extra;
    where_feed(iterator);
    const void *item = iterator->fed;
    iterator->fed = NULL;
    iterator->is_fed = false;
    return item;
}

bool where_hasnext(Iterator *wrap) {
    Iterable_Where_i *iterator = wrap->extra;
    where_feed(iterator);
    return iterator->is_fed;
}

Iterator *where_iterator(Iterable *wrap) {
    Iterable_Where *iterable = wrap->extra;
    Iterable_Where_i *iterator = malloc(sizeof(Iterable_Where_i));
    iterator->predicate = iterable->predicate;
    iterator->parent = iterable->parent->get_iterator(iterable->parent);
    iterator->fed = NULL;
    iterator->is_fed = false;
    return Iterator_new(iterator, where_next, where_hasnext);
}

////////////////////////////////////////////////////
// TakeWhile
////////////////////////////////////////////////////
typedef struct {
    Iterable *parent;
    function_predicate predicate;
} Iterable_TakeWhile;

typedef struct {
    Iterator *parent;
    function_predicate predicate;
    bool is_fed;
    bool stop;
    const void *fed;
} Iterable_TakeWhile_i;

void while_feed(Iterable_TakeWhile_i *iterator) {
    if(iterator->is_fed || iterator->stop)
        return;
    Iterator *parent = iterator->parent;
    const void *item = parent->next(parent);
    if(iterator->predicate(item)) {
        iterator->is_fed = true;
        iterator->fed = item;
    } else
        iterator->stop = true;
}

const void *while_next(Iterator *wrap) {
    Iterable_TakeWhile_i *iterator = wrap->extra;
    while_feed(iterator);
    const void *item = iterator->fed;
    iterator->fed = NULL;
    iterator->is_fed = false;
    return item;
}

bool while_hasnext(Iterator *wrap) {
    Iterable_TakeWhile_i *iterator = wrap->extra;
    while_feed(iterator);
    return iterator->is_fed;
}

Iterator *while_iterator(Iterable *wrap) {
    Iterable_TakeWhile *iterable = wrap->extra;
    Iterable_TakeWhile_i *iterator = malloc(sizeof(Iterable_TakeWhile_i));
    iterator->predicate = iterable->predicate;
    iterator->parent = iterable->parent->get_iterator(iterable->parent);
    iterator->fed = NULL;
    iterator->is_fed = false;
    return Iterator_new(iterator, while_next, while_hasnext);
}

////////////////////////////////////////////////////
// Take
////////////////////////////////////////////////////
typedef struct {
    Iterable *parent;
    size_t count;
} Iterable_Take;

typedef struct {
    Iterator *parent;
    size_t remaining;
} Iterable_Take_i;

const void *take_next(Iterator *wrap) {
    Iterable_Take_i *iterator = wrap->extra;
    if(iterator->remaining == 0)
        return NULL;
    const void *item = iterator->parent->next(iterator->parent);
    iterator->remaining--;
    return item;
}

bool take_hasnext(Iterator *wrap) {
    Iterable_Take_i *iterator = wrap->extra;
    return iterator->remaining > 0;
}

Iterator *take_iterator(Iterable *wrap) {
    Iterable_Take *iterable = wrap->extra;
    Iterable_Take_i *iterator = malloc(sizeof(Iterable_Take_i));
    iterator->parent = iterable->parent->get_iterator(iterable->parent);
    iterator->remaining = iterable->count;
    return Iterator_new(iterator, take_next, take_hasnext);
}

////////////////////////////////////////////////////
// Append/Prepend
////////////////////////////////////////////////////

typedef struct {
    bool isAppend;
    const void *item;
    Iterable *parent;
} Iterable_APend;

typedef struct {
    bool isAppend;
    bool dispatched;
    const void *item;
    Iterator *parent;
} Iterable_APend_i;

const void *APend_next(Iterator *wrap) {
    Iterable_APend_i *iterator = wrap->extra;
    Iterator *parent = iterator->parent;
    if(!iterator->isAppend && parent->has_next(parent)) {
        return parent->next(parent);
    }
    if(!iterator->dispatched) {
        const void *item = iterator->item;
        iterator->item = NULL;
        iterator->dispatched = true;
        return item;
    }
    return parent->next(parent);
}

bool APend_hasnext(Iterator *wrap) {
    Iterable_APend_i *iterator = wrap->extra;
    Iterator *parent = iterator->parent;
    return parent->has_next(parent) || !iterator->dispatched;
}

Iterator *APend_iterator(Iterable *itwrap) {
    Iterable_APend *iterable = itwrap->extra;
    Iterable_APend_i *iterator = malloc(sizeof(Iterable_APend_i));
    iterator->dispatched = false;
    iterator->isAppend = iterable->isAppend;
    iterator->parent = iterable->parent->get_iterator(iterable->parent);
    iterator->item = iterable->item;
    return Iterator_new(iterator, APend_next, APend_hasnext);
}

////////////////////////////////////////////////////
// Empty
////////////////////////////////////////////////////

const void *empty_next(Iterator *iterator) {
    return NULL;
}

bool empty_hasnext(Iterator *iterator) {
    return false;
}

Iterator *empty_iterator(Iterable *iterable) {
    return Iterator_new(NULL, empty_next, empty_hasnext);
}

////////////////////////////////////////////////////
// Append/Prepend
////////////////////////////////////////////////////

typedef struct {
    const void *default_item;
    Iterable *parent;
} Iterable_DefaultIfEmpty;

typedef struct {
    const void *default_item;
    Iterator *parent;
    bool dispatched;
} Iterable_DefaultIfEmpty_i;

const void *defaultIfEmpty_next(Iterator *wrap) {
    Iterable_DefaultIfEmpty_i *iterator = wrap->extra;
    Iterator *parent = iterator->parent;
    const void *item;
    if(!parent->has_next(parent) && !iterator->dispatched)
        item = iterator->default_item;
    else
        item = parent->next(parent);

    iterator->dispatched = true;
    iterator->default_item = NULL;
    return item;
}

bool defaultIfEmpty_hasnext(Iterator *wrap) {
    Iterable_DefaultIfEmpty_i *iterator = wrap->extra;
    Iterator *parent = iterator->parent;
    return parent->has_next(parent) || !iterator->dispatched;
}

Iterator *defaultIfEmpty_iterator(Iterable *wrap) {
    Iterable_DefaultIfEmpty *iterable = wrap->extra;
    Iterable_DefaultIfEmpty_i *iterator = malloc(sizeof(Iterable_DefaultIfEmpty_i));
    iterator->dispatched = false;
    iterator->default_item = iterable->default_item;
    iterator->parent = iterable->parent->get_iterator(iterable->parent);
    return Iterator_new(iterator, defaultIfEmpty_next, defaultIfEmpty_hasnext);
}

////////////////////////////////////////////////////
// Range
////////////////////////////////////////////////////

typedef struct {
    long long int from;
    long long int to;
} Iterable_Range;

typedef struct {
    long long int current;
    long long int last;
    bool finished;
} Iterable_Range_i;

const void *range_next(Iterator *wrap) {
    Iterable_Range_i *iterator = wrap->extra;
    if(iterator->finished)
        return NULL;
    long long int *item = malloc(sizeof(long long int));
    *item = ++iterator->current;
    if(iterator->current == iterator->last) //CHECK BOUNDARIES
        iterator->finished = true;
    return item;
}

bool range_hasnext(Iterator *wrap) {
    Iterable_Range_i *iterator = wrap->extra;
    return !iterator->finished;
}

Iterator *range_iterator(Iterable *iterable_wrap) {
    Iterable_Range *iterable = iterable_wrap->extra;
    Iterable_Range_i *iterator = malloc(sizeof(Iterable_Range_i));
    iterator->current = iterable->from;
    iterator->last = iterable->to;
    iterator->finished = false;
    return Iterator_new(iterator, range_next, range_hasnext);
}

////////////////////////////////////////////////////
// Concat
////////////////////////////////////////////////////

typedef struct {
    Iterable *first;
    Iterable *second;
} Iterable_Concat;

typedef struct {
    Iterator *first;
    Iterator *second;
} Iterable_Concat_i;

const void *concat_next(Iterator *wrap) {
    Iterable_Concat_i *iterator = wrap->extra;
    if(iterator->first->has_next(iterator->first))
        return iterator->first->next(iterator->first);
    return iterator->second->next(iterator->second);
}

bool concat_hasnext(Iterator *wrap) {
    Iterable_Concat_i *iterator = wrap->extra;
    return iterator->first->has_next(iterator->first) || iterator->second->has_next(iterator->second);
}

Iterator *concat_iterator(Iterable *iterable_wrap) {
    Iterable_Concat *iterable = iterable_wrap->extra;
    Iterable_Concat_i *iterator = malloc(sizeof(Iterable_Concat_i));
    iterator->first = iterable->first->get_iterator(iterable->first);
    iterator->second = iterable->second->get_iterator(iterable->second);
    return Iterator_new(iterator, concat_next, concat_hasnext);
}

////////////////////////////////////////////////////
// Repeat
////////////////////////////////////////////////////

typedef struct {
    const void *item;
    size_t times;
} Iterable_Repeat;

typedef struct {
    const void *item;
    size_t remaining;
} Iterable_Repeat_i;

const void *repeat_next(Iterator *wrap) {
    Iterable_Repeat_i *iterator = wrap->extra;
    const void *item = NULL;
    if(iterator->remaining != 0) {
        item = iterator->item;
        iterator->remaining--;
    }
    return item;
}

bool repeat_hasnext(Iterator *wrap) {
    Iterable_Repeat_i *iterator = wrap->extra;
    return iterator->remaining > 0;
}

Iterator *repeat_iterator(Iterable *iterable_wrap) {
    Iterable_Repeat *iterable = iterable_wrap->extra;
    Iterable_Repeat_i *iterator = malloc(sizeof(Iterable_Repeat_i));
    iterator->remaining = iterable->times;
    iterator->item = iterable->item;
    return Iterator_new(iterator, repeat_next, repeat_hasnext);
}

////////////////////////////////////////////////////
// Skip
////////////////////////////////////////////////////

typedef struct {
    Iterable *parent;
    size_t skip;
} Iterable_Skip;

typedef struct {
    Iterator *parent;
    size_t remaining;
} Iterable_Skip_i;

void skip_skip(Iterable_Skip_i *iterator) {
    while(iterator->remaining) {
        iterator->parent->next(iterator->parent);
        iterator->remaining--;
    }
}

const void *skip_next(Iterator *wrap) {
    Iterable_Skip_i *iterator = wrap->extra;
    if(iterator->remaining != 0)
        skip_skip(iterator);
    return iterator->parent->next(iterator->parent);
}

bool skip_hasNext(Iterator *wrap) {
    Iterable_Skip_i *iterator = wrap->extra;
    if(iterator->remaining != 0)
        skip_skip(iterator);
    return iterator->parent->has_next(iterator->parent);
}

Iterator *skip_iterator(Iterable *iterableWrap) {
    Iterable_Skip *iterable = iterableWrap->extra;
    Iterable_Skip_i *iterator = malloc(sizeof(Iterable_Skip_i));
    iterator->parent = iterable->parent->get_iterator(iterable->parent);
    iterator->remaining = iterable->skip;
    return Iterator_new(iterator, skip_next, skip_hasNext);
}

////////////////////////////////////////////////////
// SkipWhile
////////////////////////////////////////////////////

typedef struct {
    Iterable *parent;
    function_predicate predicate;
} Iterable_SkipWhile;

typedef struct {
    Iterator *parent;
    function_predicate predicate;
    const void *fed;
    bool isFed;
    bool isSkipped;
} Iterable_SkipWhile_i;

void skipWhile_skip(Iterable_SkipWhile_i *iterator) {
    if(iterator->isSkipped)
        return;
    Iterator *parent = iterator->parent;
    while(parent->has_next(parent) && !iterator->isFed) {
        const void *item = parent->next(parent);
        if(!iterator->predicate(item)) {
            iterator->isFed = true;
            iterator->fed = item;
        }
    }
    iterator->isSkipped = true;
}

const void *skipWhile_next(Iterator *wrap) {
    Iterable_SkipWhile_i *iterator = wrap->extra;
    skipWhile_skip(iterator);
    if(iterator->isFed) {
        const void *item = iterator->fed;
        iterator->isFed = false;
        iterator->fed = NULL;
        return item;
    } else
        return iterator->parent->next(iterator->parent);
}

bool skipWhile_hasNext(Iterator *wrap) {
    Iterable_SkipWhile_i *iterator = wrap->extra;
    skipWhile_skip(iterator);
    return iterator->isFed || iterator->parent->has_next(iterator->parent);
}

Iterator *skipWhile_iterator(Iterable *iterableWrap) {
    Iterable_SkipWhile *iterable = iterableWrap->extra;
    Iterable_SkipWhile_i *iterator = malloc(sizeof(Iterable_Skip_i));
    iterator->parent = iterable->parent->get_iterator(iterable->parent);
    iterator->predicate = iterable->predicate;
    iterator->isFed = false;
    iterator->fed = NULL;
    iterator->isSkipped = false;
    return Iterator_new(iterator, skipWhile_next, skipWhile_hasNext);
}

////////////////////////////////////////////////////
// Reverse
////////////////////////////////////////////////////

typedef struct {
    Iterable *parent;
} Iterable_Reverse;

typedef struct {
    void **ptr_item;
    size_t remaining;
} Iterable_Reverse_i;

void **realloc_ptr_buffer(void **ptr, size_t *current_capacity) {
    size_t nCapacity = *current_capacity * 2;
    if(nCapacity < *current_capacity)
        nCapacity = SIZE_MAX;
    if(nCapacity < *current_capacity)
        return NULL;
    *current_capacity = nCapacity;
    return realloc(ptr, nCapacity * sizeof(void *));
}

void reverse_feed(Iterable_Reverse_i *iterator, Iterator *parent) {
    size_t capacity = 2;
    size_t count = 0;
    void **ptr = malloc(sizeof(void *) * capacity);
    while(parent->has_next(parent)) {
        const void *item = parent->next(parent);
        if(count == capacity) {
            ptr = realloc_ptr_buffer(ptr, &capacity);
            if(ptr == NULL)
                break;
        }
        ptr[count++] = (void *) item;
    }
    iterator->ptr_item = ptr;
    iterator->remaining = ptr != NULL? count : 0;
}

bool reverse_hasNext(Iterator *wrap) {
    return ((Iterable_Reverse_i *) wrap->extra)->remaining > 0;
}

const void *reverse_next(Iterator *wrap) {
    Iterable_Reverse_i *iterator = wrap->extra;
    return iterator->remaining > 0? iterator->ptr_item[--iterator->remaining] : NULL;
}

Iterator *reverse_iterator(Iterable *iterable_wrap) {
    Iterable_Reverse *iterable = iterable_wrap->extra;
    Iterable_Reverse_i *iterator = malloc(sizeof(Iterable_Reverse_i));
    Iterable *parent = iterable->parent;
    reverse_feed(iterator, parent->get_iterator(parent));
    return Iterator_new(iterator, reverse_next, reverse_hasNext);
}

////////////////////////////////////////////////////
// Factory
////////////////////////////////////////////////////

Iterable *it_new(const void *data, size_t item_size, size_t count) {
    Iterable_Array *iterable = malloc(sizeof(Iterable_Array));
    iterable->array = data;
    iterable->item_size = item_size;
    iterable->count = count;
    return Iterable_new(iterable, array_iterator);
}

Iterable *it_where(Iterable *parent, function_predicate predicate) {
    Iterable_Where *iterable = malloc(sizeof(Iterable_Where));
    iterable->predicate = predicate;
    iterable->parent = parent;
    return Iterable_new(iterable, where_iterator);
}

Iterable *it_select(Iterable *parent, function_selector selector) {
    Iterable_Select *iterable = malloc(sizeof(Iterable_Select));
    iterable->parent = parent;
    iterable->selector = selector;
    return Iterable_new(iterable, select_iterator);
}

Iterable *it_take(Iterable *parent, size_t count) {
    Iterable_Take *iterable = malloc(sizeof(Iterable));
    iterable->parent = parent;
    iterable->count = count;
    return Iterable_new(iterable, take_iterator);
}

Iterable *it_takeWhile(Iterable *parent, function_predicate predicate) {
    Iterable_TakeWhile *iterable = malloc(sizeof(Iterable_TakeWhile));
    iterable->parent = parent;
    iterable->predicate = predicate;
    return Iterable_new(iterable, while_iterator);
}

size_t it_count(Iterable *iterable) {
    size_t count = 0;
    IT_ITERATE(iterable, void, count++;);
    return count;
}

bool it_all(Iterable *iterable, function_predicate predicate) {
    IT_ITERATE(iterable, void, if(!predicate(___value)) return false;);
    return true;
}

bool it_any(Iterable *iterable, function_predicate predicate) {
    IT_ITERATE(iterable, void, if(predicate(___value)) return true;);
    return false;
}

const void *it_at(Iterable *iterable, size_t index) {
    size_t cindex = 0;
    IT_ITERATE(iterable, void, if(cindex++ == index) return ___value;);
    return NULL;
}

const void *it_last(Iterable *iterable) {
    const void *item = NULL;
    IT_ITERATE(iterable, void, item = ___value;);
    return item;
}

const void *it_first(Iterable *iterable) {
    const void *item = NULL;
    Iterator *it = iterable->get_iterator(iterable);
    if(it->has_next(it))
        item = it->next(it);
    return item;
}

void *it_accumulate(Iterable *iterable, const void *seed, function_accumulator accumulator) {
    void *ptr = NULL;
    bool hasValue = false;
    IT_ITERATE(iterable, void,
               if(!hasValue) //avoid removing const qualifier.
                   ptr = accumulator(seed, ___value);
               else
                   ptr = accumulator(ptr, ___value);
                       hasValue = true;
    );
    return ptr;
}

Iterable *it_createAPend(Iterable *parent, const void *item, bool isAppend) {
    Iterable_APend *iterable = malloc(sizeof(Iterable_APend));
    iterable->isAppend = isAppend;
    iterable->item = item;
    iterable->parent = parent;
    return Iterable_new(iterable, APend_iterator);
}

Iterable *it_append(Iterable *parent, const void *item) {
    return it_createAPend(parent, item, true);
}

Iterable *it_prepend(Iterable *parent, const void *item) {
    return it_createAPend(parent, item, false);
}

Iterable *it_defaultIfEmpty(Iterable *parent, const void *item) {
    Iterable_DefaultIfEmpty *iterable = malloc(sizeof(Iterable_DefaultIfEmpty));
    iterable->parent = parent;
    iterable->default_item = item;
    return Iterable_new(iterable, defaultIfEmpty_iterator);
}

Iterable *it_range(long long int from, long long int to) {
    if(to < from)
        return NULL;
    Iterable_Range *iterable = malloc(sizeof(Iterable_Range));
    iterable->to = to;
    iterable->from = from;
    return Iterable_new(iterable, range_iterator);
}

Iterable *it_empty() {
    return Iterable_new(NULL, empty_iterator);
}

Iterable *it_repeat(const void *item, size_t times) {
    Iterable_Repeat *iterable = malloc(sizeof(Iterable_Repeat));
    iterable->item = item;
    iterable->times = times;
    return Iterable_new(iterable, repeat_iterator);
}

bool it_contains(Iterable *iterable, const void *item, function_comparator comparator) {
    IT_ITERATE(iterable, void, if(comparator(item, ___value)) return true;);
    return false;
}

Iterable *it_concat(Iterable *first, Iterable *second) {
    Iterable_Concat *iterable = malloc(sizeof(Iterable_Concat));
    iterable->first = first;
    iterable->second = second;
    return Iterable_new(iterable, concat_iterator);
}

bool it_hasnext(Iterator *iterator) {
    return iterator->has_next(iterator);
}

const void *it_next(Iterator *iterator) {
    return iterator->next(iterator);
}

Iterable *it_skip(Iterable *parent, size_t skip) {
    Iterable_Skip *iterable = malloc(sizeof(Iterable_Skip));
    iterable->parent = parent;
    iterable->skip = skip;
    return Iterable_new(iterable, skip_iterator);
}

Iterable *it_skipWhile(Iterable *parent, function_predicate predicate) {
    Iterable_SkipWhile *iterable = malloc(sizeof(Iterable_SkipWhile));
    iterable->parent = parent;
    iterable->predicate = predicate;
    return Iterable_new(iterable, skipWhile_iterator);
}

Iterator *it_iterator(Iterable *iterable) {
    return iterable->get_iterator(iterable);
}


Iterable *it_reverse(Iterable *parent) {
    Iterable_Reverse *iterable = malloc(sizeof(Iterable_Reverse));
    iterable->parent = parent;
    return Iterable_new(iterable, reverse_iterator);
}

//BUFFER IMPL

void *realloc_buffer(void *ptr, size_t *current_capacity, size_t item_size) {
    size_t nCapacity = *current_capacity * 2;
    if(nCapacity < *current_capacity)
        nCapacity = SIZE_MAX;
    if(nCapacity < *current_capacity)
        return NULL;
    *current_capacity = nCapacity;
    return realloc(ptr, nCapacity * item_size);
}

void *it_toArray(Iterable *iterable, size_t item_size, size_t *total_count) {
    size_t capacity = 2;
    size_t count = 0;
    void *ptr = malloc(item_size * capacity);
    Iterator *iterator = iterable->get_iterator(iterable);
    while(iterator->has_next(iterator)) {
        const void *item = iterator->next(iterator);
        if(count == capacity) {
            ptr = realloc_buffer(ptr, &capacity, item_size);
            if(ptr == NULL) return NULL;
        }
        void *ptrToCopy = ptr + (item_size * count);
        memcpy(ptrToCopy, item, item_size);
        count++;
    }
    if(count != capacity) {
        void *truncated = malloc(count * item_size);
        ptr = memcpy(truncated, ptr, count * item_size);
    }
    *total_count = count;
    return ptr;
}
