#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#endif // ARRAY_H

// Included once per new array type

#ifndef ARRAY_NAME
#error "Must define ARRAY_NAME"
#endif

#ifndef ARRAY_TYPE
#error "Must define ARRAY_TYPE"
#endif

#ifndef DEFAULT_ARRAY_SIZE
#define NO_DEFAULT_ARRAY_SIZE
#define DEFAULT_ARRAY_SIZE 8
#endif

#ifndef ARRAY_GROWTH_FUNC
#define ARRAY_GROWTH_FUNC(x) ((x) * 3 / 2)
#endif

#ifndef ARRAY_MALLOC
#define ARRAY_MALLOC malloc
#define ARRAY_MALLOC_DEFINED
#endif

#ifndef ARRAY_REALLOC
#define ARRAY_REALLOC realloc
#define ARRAY_REALLOC_DEFINED
#endif

#ifndef ARRAY_FREE
#define ARRAY_FREE free
#define ARRAY_FREE_DEFINED
#endif

#define CONCAT_(a, b) a ## b
#define CONCAT(a, b) CONCAT_(a, b)
#define ARRAY_FUNC(func) CONCAT(ARRAY_NAME, _##func)

typedef struct {
    size_t n, m;
    ARRAY_TYPE *a;
} ARRAY_NAME;

static inline ARRAY_NAME *ARRAY_FUNC(new_size)(size_t size) {
    ARRAY_NAME *array = malloc(sizeof(ARRAY_NAME));
    if (array == NULL) return NULL;
    array->n = array->m = 0;
    array->a = ARRAY_MALLOC((size > 0 ? size : 1) * sizeof(ARRAY_TYPE));
    if (array->a == NULL) return NULL;
    array->m = size;
    return array;
}

static inline ARRAY_NAME *ARRAY_FUNC(new)(void) {
    return ARRAY_FUNC(new_size)(DEFAULT_ARRAY_SIZE);
}

static inline ARRAY_NAME *ARRAY_FUNC(new_size_fixed)(size_t size) {
    ARRAY_NAME *array = ARRAY_FUNC(new_size)(size);
    if (array == NULL) return NULL;
    array->n = size;
    return array;
}

static inline bool ARRAY_FUNC(resize)(ARRAY_NAME *array, size_t size) {
    if (size <= array->m) return true;
    #ifndef ARRAY_REALLOC_NEEDS_PREV_SIZE
    ARRAY_TYPE *ptr = ARRAY_REALLOC(array->a, sizeof(ARRAY_TYPE) * size);
    #else
    ARRAY_TYPE *ptr = ARRAY_REALLOC(array->a, sizeof(ARRAY_TYPE) * array->m, sizeof(ARRAY_TYPE) * size);
    #endif
    if (ptr == NULL) return false;
    array->a = ptr;
    array->m = size;
    return true;
}

static inline bool ARRAY_FUNC(resize_to_fit)(ARRAY_NAME *array, size_t needed_capacity) {
    size_t cap = array->m;
    if (cap >= needed_capacity) return true;
    if (cap == 0) cap = DEFAULT_ARRAY_SIZE;
    size_t prev_cap = cap;
    while (cap < needed_capacity) {
        cap = ARRAY_GROWTH_FUNC(prev_cap);
        if (cap == prev_cap) cap++;
        prev_cap = cap;
    }
    return ARRAY_FUNC(resize)(array, cap);
}

static inline bool ARRAY_FUNC(resize_fixed)(ARRAY_NAME *array, size_t size) {
    if (!ARRAY_FUNC(resize)(array, size)) return false;
    array->n = size;
    return true;
}

static size_t ARRAY_FUNC(capacity)(ARRAY_NAME *array) {
    return array->m;
}

static size_t ARRAY_FUNC(len)(ARRAY_NAME *array) {
    return array->n;
}

static inline ARRAY_TYPE ARRAY_FUNC(get_unchecked)(ARRAY_NAME *array, size_t index) {
    return array->a[index];
}

static inline bool ARRAY_FUNC(get)(ARRAY_NAME *array, size_t index, ARRAY_TYPE *result) {
    if (index >= array->n) return false;
    *result = array->a[index];
    return true;
}

static inline bool ARRAY_FUNC(set)(ARRAY_NAME *array, size_t index, ARRAY_TYPE value) {
    if (index >= array->n) return false;
    array->a[index] = value;
    return true;
}

static inline void ARRAY_FUNC(set_unchecked)(ARRAY_NAME *array, size_t index, ARRAY_TYPE value) {
    array->a[index] = value;
}

static inline bool ARRAY_FUNC(push)(ARRAY_NAME *array, ARRAY_TYPE value) {
    size_t cap = array->m;
    if (array->n >= cap) {
        if (!ARRAY_FUNC(resize_to_fit)(array, array->n + 1)) return false;
    }
    array->a[array->n++] = value;
    return true;
}

static inline bool ARRAY_FUNC(extend)(ARRAY_NAME *array, ARRAY_TYPE *values, size_t n) {
    size_t new_size = array->n + n;
    size_t current_capacity = array->m;
    if (!ARRAY_FUNC(resize_to_fit)(array, new_size)) return false;
    memcpy(array->a + array->n, values, n * sizeof(ARRAY_TYPE));
    array->n = new_size;
    return true;
}

static inline bool ARRAY_FUNC(concat)(ARRAY_NAME *array, ARRAY_NAME *other) {
    return ARRAY_FUNC(extend)(array, other->a, other->n);
}

static inline bool ARRAY_FUNC(empty)(ARRAY_NAME *array) {
    return array->n == 0;
}

static inline bool ARRAY_FUNC(pop)(ARRAY_NAME *array, ARRAY_TYPE *result) {
    if (ARRAY_FUNC(empty)(array)) return false;
    *result = array->a[--array->n];
    return true;
}

static inline void ARRAY_FUNC(clear)(ARRAY_NAME *array) {
    array->n = 0;
}

static inline bool ARRAY_FUNC(copy)(ARRAY_NAME *dst, ARRAY_NAME *src, size_t n) {
    bool ret = true;
    if (dst->m < n) ret = ARRAY_FUNC(resize)(dst, n);
    if (!ret) return false;
    memcpy(dst->a, src->a, n * sizeof(ARRAY_TYPE));
    dst->n = n;
    return ret;
}
static inline ARRAY_NAME *ARRAY_FUNC(new_copy)(ARRAY_NAME *array, size_t n) {
    ARRAY_NAME *cpy = ARRAY_FUNC(new_size)(n);
    if (!ARRAY_FUNC(copy)(cpy, array, n)) return NULL;
    return cpy;
}

static inline ARRAY_NAME *ARRAY_FUNC(new_value)(size_t n, ARRAY_TYPE value) {
    ARRAY_NAME *array = ARRAY_FUNC(new_size)(n);
    if (array == NULL) return NULL;
    for (size_t i = 0; i < n; i++) {
        array->a[i] = value;
    }
    array->n = n;
    return array;
}

#ifdef ARRAY_IS_NUMERIC
static inline ARRAY_NAME *ARRAY_FUNC(new_ones)(size_t n) {
    return ARRAY_FUNC(new_value)(n, (ARRAY_TYPE)1);
}

static inline ARRAY_NAME *ARRAY_FUNC(new_zeros)(size_t n) {
    ARRAY_NAME *array = ARRAY_FUNC(new_size)(n);
    if (array == NULL) return NULL;
    ARRAY_FUNC(zero)(array->a, n);
    array->n = n;
    return array;
}
#endif

static inline void ARRAY_FUNC(destroy)(ARRAY_NAME *array) {
    if (array == NULL) return;
    if (array->a != NULL) {
    #ifdef ARRAY_FREE_DATA
        for (size_t i = 0; i < array->n; i++) {
            ARRAY_FREE_DATA(array->a[i]);
        }
    #endif
        ARRAY_FREE(array->a);
    }
    free(array);
}

#define INTROSORT_NAME ARRAY_NAME
#define INTROSORT_TYPE ARRAY_TYPE
#ifdef ARRAY_SORT_LT
#define INTROSORT_LT(a, b) ARRAY_SORT_LT(a, b)
#endif
#include "sort/introsort.h"
#ifdef INTROSORT_LT
#undef INTROSORT_LT
#endif
#undef INTROSORT_NAME
#undef INTROSORT_TYPE

static inline bool ARRAY_FUNC(sort)(ARRAY_NAME *array) {
    if (array == NULL || array->n <= 1) return false;
    ARRAY_FUNC(introsort)(array->n, array->a);
    return true;
}

static inline bool ARRAY_FUNC(sort_reverse)(ARRAY_NAME *array) {
    if (array == NULL || array->n <= 1) return false;
    ARRAY_FUNC(introsort_reverse)(array->n, array->a);
    return true;
}

#define ARRAY_INDEX_NAME CONCAT(ARRAY_NAME, _indices)
#define ARRAY_INDEX_FUNC(func) CONCAT(ARRAY_NAME, _indices_##func)

#define INTROSORT_NAME ARRAY_INDEX_NAME
#define INTROSORT_TYPE size_t
#ifdef ARRAY_SORT_LT
#define INTROSORT_LT(a, b, aux) ARRAY_SORT_LT(((aux)[(a)], (aux)[(b)]))
#endif
#define INTROSORT_AUX_TYPE ARRAY_TYPE
#include "sort/introsort.h"
#ifdef INTROSORT_LT
#undef INTROSORT_LT
#endif
#undef INTROSORT_NAME
#undef INTROSORT_TYPE
#undef INTROSORT_AUX_TYPE

#undef ARRAY_INDEX_NAME
#undef ARRAY_INDEX_TYPE


static inline bool ARRAY_FUNC(argsort)(ARRAY_NAME *array, size_t *indices) {
    if (array == NULL) return NULL;
    if (indices == NULL) return NULL;
    for (size_t i = 0; i < array->n; i++) {
        indices[i] = i;
    }
    ARRAY_INDEX_FUNC(introsort)(array->n, indices, array->a);
    return true;
}

static inline bool ARRAY_FUNC(argsort_reverse)(ARRAY_NAME *array, size_t *indices) {
    if (array == NULL) return false;
    if (indices == NULL) return false;
    for (size_t i = 0; i < array->n; i++) {
        indices[i] = i;
    }
    ARRAY_INDEX_FUNC(introsort_reverse)(array->n, indices, array->a);
    return true;
}

#undef CONCAT_
#undef CONCAT
#undef ARRAY_FUNC
#ifdef NO_DEFAULT_ARRAY_SIZE
#undef NO_DEFAULT_ARRAY_SIZE
#undef DEFAULT_ARRAY_SIZE
#endif
#ifdef ARRAY_MALLOC_DEFINED
#undef ARRAY_MALLOC
#undef ARRAY_MALLOC_DEFINED
#endif
#ifdef ARRAY_REALLOC_DEFINED
#undef ARRAY_REALLOC
#undef ARRAY_REALLOC_DEFINED
#endif
#ifdef ARRAY_FREE_DEFINED
#undef ARRAY_FREE
#undef ARRAY_FREE_DEFINED
#endif
