# cinq - C integrated queries
cinq provides iterators and array queries to c. This library ships with array support out of the box, but can be extended to your own custom structures.

**NOTE: This is a concept library and MUST not be used in production.** See Limitations section.
## Usage example
```C
bool my_predicate(const void *item){
    return *((int*)item) < 6;
}
int numbers[] = {1, 3, 5, 7, 9, 2, 4, 6, 8, 10};
Iterable *myIterable = IT_NEW(numbers);

Iterable *myQuery = it_where(myIterable, my_predicate);
IT_ITERATE(myQuery, int, printf("%d ", *___value););
printf("\n");
Iterable *myQuery2 = it_takeWhile(myIterable, my_predicate);
IT_ITERATE(myQuery2, int, printf("%d ", *___value););
```
stdout:
```
1 2 3 4 5
1 3 5
```

# Create your own iterable extension
You can extend the library to use your own structure (just like an interface implementation)
1. First create your own structure
```C
typedef struct{
    void *items;
    size_t count;
    size_t item_size;
}List;
```
2. Define your iterable structure. The iterable structure should contain all data needed to create an iterator.
```C
typedef struct{
    void *items;
    size_t count;
    size_t item_size;
}List_Iterable;
```
3. Define your iterator structure, that is, any data needed to keep track of the current and next element.
```C
typedef struct{
    void *items;
    size_t count;
    size_t item_size;
    size_t current_index;
}List_Iterator;
```
4. Then define the factory functions
```C
const void *list_next(Iterator *wrapper){
    List_Iterator *iterator = wrapper->extra;
    return iterator->items[current_index++ * iterator->item_size];
}
bool list_hasNext(Iterator *wrapper){
    List_Iterator *iterator = wrapper->extra;
    return iterator->current_index < iterator->count;
}
Iterator *list_createIterator(Iterable *wrapper){
    List_Iterable *iterable = wrapper->extra;
    List_Iterator *iterator = malloc(sizeof(List_Iterator));
    iterator->items = iterable->items;
    iterator->count = iterable->count;
    iterator->item_size = iterable->item_size;
    iterator->current_index = 0;
    return Iterator_new(iterator, list_next, list_hasNext);
}
Iterator *list_createIterable(List *list){
   List_Iterable *iterable = malloc(sizeof(List_Iterable));
   iterable->items = list->items;
   iterable->count = list->count;
   iterable->item_size = list->item_size;
   return Iterable_new(iterable, list_createIterator);
}
```
5. Use it with cinq
```C
List myList = /* ... */;
Iterable *myIterable = list_createIterable(&myList);
Iterable *myQuery = it_where(myIterable, predicate);
```
# Limitations
This is a concept library and MUST not be used in production.
## Using nested defined macros
Violation:
```C
int numbers[] = {1, 2, 3};
Iterable *iterable = IT_NEW(numbers);
IT_ITERATE(iterable, int, int max = IT_MAX(iterable););
```
The macro-expanded code is:
```C
int numbers[] = {1, 2, 3};
Iterable *iterable = it_new(numbers, sizeof(int), sizeof(numbers) / sizeof(int));
do{
    Iterable *___iterable = iterable;
    Iterator *___iterator = iterable->get_iterator(___iterable);
    const int *___value;
    while(___iterator->has_next(___iterator)){
        ___value = ___iterator->next(___iterator);
        int max;
        int ___max = 0;
        bool ___hasElements = false;
        do{
            Iterable *___iterable = iterable;
            Iterator *___iterator = iterable->get_iterator(___iterable);
            const int *___value;
            while(___iterator->has_next(___iterator)){
                ___max = !_hasElements || ___max < *___value? *___value : ___max;
                hasElements = true;
            }while(0);
        max = ___max;
        }while(0);
    }
}while(0);
```
Resulting in multiple same name variable declarations.

## Iterators and iterables cannot be freed
Even if you call `void free(void*)` on an `Iterable*` or an `Iterator*`, your own implementation won't be freed creating nasty memory leaks.
