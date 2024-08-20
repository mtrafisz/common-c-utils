/*
Very simple dynamic array (vector?) implementation in pure C.

Usage:

```c
#include <array.h>
#include <stdio.h>

int compare_fn(const void* a, const void* b) {
    return *((int*)a) - *((int*)b);
}

Array reverse(ArrayPtr a) {
    Array result = array_ex(a->sizeof_element, a->size, a->allocator);

    for (uint32_t i = array_size(a) - 1; i > 0; --i) {
        array_append(&result, array_at(a, i));
    }

    return result;
}

int main(void) {
    Array a = array(int, STD_ALLOCATOR);

    for (int i = 'a'; i <= 'z'; ++i) {
        array_append(&a, &i);
    }

    Array b = reverse(&a);

    array_destroy(&a);

    array_sort(&b, compare_fn);

    ArrayIterator b_iter = iterator(&b);
    int* b_val;

    while ((b_val = iterator_next(&b_iter))) {
        printf("%d ", *b_val);
    }

    array_destroy(&b);
    return 0;
}
```

WARNING - whole implementation is filled with not-null assertions,
          so any error in Your part of the code will cause fatal crash.
          It's still better than SIGSEV I think.
*/

#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define _CAR_ARR_DEFAULT_SIZE 16
/**
 * Macro to provide basic type hinting.
 *
 * Ex. You can write
 * `Array(int) create_some_int_array(size_t size) {...}`
 * To signify, that array will contain pointers to ints
 *
 * The hint itself doesn't mean anything to code or compiler,
 * You can as well write `Array(poopoodoodoo)` - its just
 * a visual change to improve readability
 */
#define Array(T) Array

typedef void* (*alloc_func)(size_t);
typedef void* (*realloc_func)(void*, size_t, size_t);
typedef void  (*dealloc_func)(void*);

typedef struct _car_allocator_s {
    alloc_func   alloc;
    realloc_func realloc;
    dealloc_func dealloc;
} Allocator;

extern const Allocator STD_ALLOCATOR;

typedef struct _car_sarr_s {
    uint32_t    size;
    uint32_t    capacity;
    size_t      sizeof_element;
    void*       data;
    Allocator   allocator;
} Array, *ArrayPtr;

/**
 * @warning no memory is allocated for iterator or its members - pointers point directly to Array memory. DO NOT FREE THEM.
 */
typedef struct _car_sarr_it_s {
    void* current;
    void* end;
    size_t sizeof_element;
} ArrayIterator, *ArrayIteratorPtr;

/**
 * Create new Array structure with given capacity
 *
 * @param sizeof_element size of element of array in bytes
 * @param capacity initial capacity of array
 * @param allocator allocator structure, that all dynamic allocations will go through
 * @return initialized Array structure
 * @warning capacity can't be set to 0
 */
extern Array array_ex(size_t sizeof_element, uint32_t capacity, Allocator allocator);

/**
 * Create new Array strucute with default capacity
 *
 * @param T type of array element
 * @param a allocator structure, that all dynamic allocations will go through
 * @return initialized Array structure
 */
#define array(T, a) array_ex(sizeof(T), _CAR_ARR_DEFAULT_SIZE, a)

/**
 * Create a clone of given array, with all data copied
 *
 * @param a array to clone
 * @return cloned array
 */
Array array_clone(const ArrayPtr a);

/**
 * Set array size to `new_size`.
 *
 * If `new_size` is greater than current capacity, array will be resized to exactly `new_size`.
 * In that case, array size will equal array capacity.
 *
 * @param a array to resize
 * @param new_size new size of array
 */
void array_resize(ArrayPtr a, uint32_t new_size);

/**
 * Append new element to array
 *
 * This function copies element pointed by p to the end of array.
 * Caller is still the owner of the memory pointed by `p`.
 *
 * @param a array to append to
 * @param p pointer to element to append
 */
void array_append(ArrayPtr a, const void* p);

/**
 * Set element at index `i` to `p`
 *
 * This function copies element pointed to by `p` to the `i`-th element of array.
 * Caller is still the owner of the memory pointed by `p`.
 *
 * This function asserts that `i` is less than array size.
 *
 * @param a array to set element in
 * @param i index of element to set
 * @param p pointer to element to set
 */
void array_set(ArrayPtr a, uint32_t i, const void* p);

/**
 * Get pointer to element at index `i`
 *
 * This function asserts that `i` is less than array capacity.
 *
 * @param a array to get element from
 * @param i index of element to get
 * @return pointer to element at index `i`
 */
void* array_at(const ArrayPtr a, uint32_t i);

/**
 * Get size of array.
 *
 * Calling this function in just equivalent to accessing `size` field of Array structure.
 *
 * @param a array to get size of
 * @return size of array
 */
uint32_t array_size(const ArrayPtr a);

/**
 * Shrink array capacity to exactly array size
 *
 * @param a array to shrink
 */
void array_shrink(ArrayPtr a);

/**
 * Destroy array
 *
 * This function deallocates all memory used by array using functions from `allocator` field.
 *
 * @param a array to destroy
 * @warning this function doesn't free memory pointed by elements of array, nor the array itself.
 */
void array_destroy(ArrayPtr a);

/**
 * Create new iterator for array
 *
 * @param a array to create iterator for
 * @return iterator for array
 */
ArrayIterator iterator(const ArrayPtr a);

/**
 * Access next element of iterator
 *
 * @param i iterator to access next element of
 * @return pointer to next element of iterator, or NULL if there are no more elements
 * @warning returned pointer must not be freed. It points directly to array memory.
 */
void* iterator_next(ArrayIteratorPtr i);

/**
 * Concat contents of array `b` to array `a`
 *
 * @param a array to concat to
 * @param b array to get elements from and add to array `a`
 */
void array_combine(ArrayPtr a, const ArrayPtr b);

/**
 * Sort array elements using qsort
 *
 * @param a array to sort
 * @param cmp compare function
 */
void array_sort(ArrayPtr a, int(*compare)(const void* a, const void* b));

#endif
