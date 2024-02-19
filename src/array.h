#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "aligned/aligned.h"

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
    array->a = malloc((size > 0 ? size : 1) * sizeof(ARRAY_TYPE));
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

static inline ARRAY_NAME *ARRAY_FUNC(new_aligned)(size_t size, size_t alignment) {
    ARRAY_NAME *array = malloc(sizeof(ARRAY_NAME));
    if (array == NULL) return NULL;
    array->n = array->m = 0;
    array->a = aligned_malloc(size * sizeof(ARRAY_TYPE), alignment);
    if (array->a == NULL) return NULL;
    array->m = size;
    return array;
}

static inline bool ARRAY_FUNC(resize)(ARRAY_NAME *array, size_t size) {
    if (size <= array->m) return true;
    ARRAY_TYPE *ptr = realloc(array->a, sizeof(ARRAY_TYPE) * size);
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

static inline bool ARRAY_FUNC(resize_aligned)(ARRAY_NAME *array, size_t size, size_t alignment) {
    if (size <= array->m) return true;
    ARRAY_TYPE *ptr = aligned_resize(array->a, sizeof(ARRAY_TYPE) * array->m, sizeof(ARRAY_TYPE) * size, alignment);
    if (ptr == NULL) return false;
    array->a = ptr;
    array->m = size;
    return true;
}

static inline bool ARRAY_FUNC(resize_fixed)(ARRAY_NAME *array, size_t size) {
    if (!ARRAY_FUNC(resize)(array, size)) return false;
    array->n = size;
    return true;
}

static inline bool ARRAY_FUNC(resize_fixed_aligned)(ARRAY_NAME *array, size_t size, size_t alignment) {
    if (!ARRAY_FUNC(resize_aligned)(array, size, alignment)) return false;
    array->n = size;
    return true;
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

#ifdef ARRAY_IS_NUMERIC
static inline ARRAY_NAME *ARRAY_FUNC(new_value)(size_t n, ARRAY_TYPE value) {
    ARRAY_NAME *array = ARRAY_FUNC(new_size)(n);
    if (array == NULL) return NULL;
    for (size_t i = 0; i < n; i++) {
        array->a[i] = value;
    }
    array->n = n;
    return array;
}

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
        free(array->a);
    }
    free(array);
}

static inline void ARRAY_FUNC(destroy_aligned)(ARRAY_NAME *array) {
    if (array == NULL) return;
    if (array->a != NULL) {
    #ifdef ARRAY_FREE_DATA
        for (size_t i = 0; i < array->n; i++) {
            ARRAY_FREE_DATA(array->a[i]);
        }
    #endif
        aligned_free(array->a);
    }
    free(array);
}

#undef CONCAT_
#undef CONCAT
#undef ARRAY_FUNC
#ifdef NO_DEFAULT_ARRAY_SIZE
#undef NO_DEFAULT_ARRAY_SIZE
#undef DEFAULT_ARRAY_SIZE
#endif
