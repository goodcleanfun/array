#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
#include <malloc.h>
static inline void *aligned_malloc(size_t size, size_t alignment) {
    return _aligned_malloc(size, alignment);
}
static inline void *aligned_resize(void *p, size_t old_size, size_t new_size, size_t alignment) {
    return _aligned_realloc(p, new_size, alignment);
}
static inline void aligned_free(void *p) {
    _aligned_free(p);
}
#else
#include <stdlib.h>
static inline void *aligned_malloc(size_t size, size_t alignment)
{
    void *p;
    int ret = posix_memalign(&p, alignment, size);
    return (ret == 0) ? p : NULL;
}
static inline void *aligned_resize(void *p, size_t old_size, size_t new_size, size_t alignment)
{
    if ((alignment == 0) || ((alignment & (alignment - 1)) != 0) || (alignment < sizeof(void *))) {
        return NULL;
    }

    if (p == NULL) {
        return NULL;
    }

    void *p1 = aligned_malloc(new_size, alignment);
    if (p1 == NULL) {
        free(p);
        return NULL;
    }

    memcpy(p1, p, old_size);
    free(p);
    return p1;
}
static inline void aligned_free(void *p)
{
    free(p);
}
#endif

#ifdef _MSC_VER
#define MIE_ALIGN(x) __declspec(align(x))
#else
#define MIE_ALIGN(x) __attribute__((aligned(x)))
#endif

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

#ifndef ARRAY_GROW
#define ARRAY_GROW(x) ((x) * 3 / 2)
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
        size_t new_cap = cap > 0 ? ARRAY_GROW(cap) : DEFAULT_ARRAY_SIZE;
        ARRAY_TYPE *ptr = realloc(array->a, sizeof(ARRAY_TYPE) * new_cap);
        if (ptr == NULL) {
            return false;
        }
        array->a = ptr;
        array->m = new_cap;
    }
    array->a[array->n++] = value;
    return true;
}

static inline bool ARRAY_FUNC(extend)(ARRAY_NAME *array, ARRAY_NAME *other) {
    bool ret = false;
    size_t new_size = array->n + other->n;
    size_t current_capacity = array->m;
    if (new_size > current_capacity) {
        size_t capacity = current_capacity;
        while (capacity < new_size) {
            capacity = capacity * 3 / 2;
        }
        ret = ARRAY_FUNC(resize)(array, capacity);
        if (!ret) return false;
    }
    memcpy(array->a + array->n, other->a, other->n * sizeof(ARRAY_TYPE));
    array->n = new_size;
    return ret;
}

static inline bool ARRAY_FUNC(pop)(ARRAY_NAME *array, ARRAY_TYPE *result) {
    if (array->n == 0) return false;
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
