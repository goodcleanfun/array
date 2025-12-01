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

#ifndef ARRAY_DATA_MALLOC
#define ARRAY_DATA_MALLOC malloc
#define ARRAY_DATA_MALLOC_DEFINED
#endif

#ifndef ARRAY_DATA_REALLOC
#define ARRAY_DATA_REALLOC realloc
#define ARRAY_DATA_REALLOC_DEFINED
#endif

#ifndef ARRAY_DATA_FREE
#define ARRAY_DATA_FREE free
#define ARRAY_DATA_FREE_DEFINED
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

static inline bool ARRAY_FUNC(init_size)(ARRAY_NAME *array, size_t size) {
    if (array == NULL) return false;
    array->n = array->m = 0;
    array->a = ARRAY_DATA_MALLOC((size > 0 ? size : 1) * sizeof(ARRAY_TYPE));
    if (array->a == NULL) return false;
    array->m = size;
    return true;
}

static inline bool ARRAY_FUNC(init_size_fixed)(ARRAY_NAME *array, size_t size) {
    if (array == NULL) return false;
    if (!ARRAY_FUNC(init_size)(array, size)) return false;
    array->n = size;
    return true;
}

static inline bool ARRAY_FUNC(init)(ARRAY_NAME *array) {
    return ARRAY_FUNC(init_size)(array, DEFAULT_ARRAY_SIZE);
}

static inline ARRAY_NAME *ARRAY_FUNC(new_size)(size_t size) {
    ARRAY_NAME *array = ARRAY_MALLOC(sizeof(ARRAY_NAME));
    if (array == NULL) return NULL;
    if (!ARRAY_FUNC(init_size)(array, size)) return NULL;
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
    #ifndef ARRAY_DATA_REALLOC_NEEDS_PREV_SIZE
    ARRAY_TYPE *ptr = ARRAY_DATA_REALLOC(array->a, sizeof(ARRAY_TYPE) * size);
    #else
    ARRAY_TYPE *ptr = ARRAY_DATA_REALLOC(array->a, sizeof(ARRAY_TYPE) * array->m, sizeof(ARRAY_TYPE) * size);
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

static size_t ARRAY_FUNC(size)(ARRAY_NAME *array) {
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

static inline bool ARRAY_FUNC(at)(ARRAY_NAME *array, size_t index, ARRAY_TYPE **result) {
    if (index >= array->n) return false;
    *result = array->a + index;
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

static inline bool ARRAY_FUNC(copy)(ARRAY_NAME *dst, ARRAY_NAME  *src, size_t n) {
    if (dst == NULL || src == NULL || n == 0 || dst == src || src->a == dst->a) return false;
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

static inline void ARRAY_FUNC(destroy_data)(ARRAY_NAME *array) {
    if (array == NULL) return;
    if (array->a != NULL) {
    #ifdef ARRAY_DATA_FREE_ELEMENT
        for (size_t i = 0; i < array->n; i++) {
            ARRAY_DATA_FREE_ELEMENT(array->a[i]);
        }
    #endif
        ARRAY_DATA_FREE(array->a);
    }
}

static inline void ARRAY_FUNC(destroy)(ARRAY_NAME *array) {
    if (array == NULL) return;
    ARRAY_FUNC(destroy_data)(array);
    ARRAY_FREE(array);
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
#ifdef ARRAY_DATA_MALLOC_DEFINED
#undef ARRAY_DATA_MALLOC
#undef ARRAY_DATA_MALLOC_DEFINED
#endif
#ifdef ARRAY_DATA_REALLOC_DEFINED
#undef ARRAY_DATA_REALLOC
#undef ARRAY_DATA_REALLOC_DEFINED
#endif
#ifdef ARRAY_FREE_DEFINED
#undef ARRAY_FREE
#undef ARRAY_FREE_DEFINED
#endif
