#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>
#include <stdlib.h>

#endif // ARRAY_H

// Included once per new array type

#ifndef ARRAY_NAME
#error "Must define ARRAY_NAME"
#endif

#ifndef ARRAY_TYPE
#error "Must define ARRAY_TYPE"
#endif

#define ARRAY_SORT_CONCAT_(a, b) a ## b
#define ARRAY_SORT_CONCAT(a, b) ARRAY_SORT_CONCAT_(a, b)
#define ARRAY_SORT_FUNC(name) ARRAY_SORT_CONCAT(ARRAY_NAME, _##name)

#define ARRAY_INDEX_NAME ARRAY_SORT_CONCAT(ARRAY_NAME, _indices)
#define ARRAY_SORT_INDEX_FUNC(func) ARRAY_SORT_CONCAT(ARRAY_NAME, _indices_##func)

#define INTROSORT_NAME ARRAY_NAME
#define INTROSORT_TYPE ARRAY_TYPE
#ifdef ARRAY_SORT_LT
#define INTROSORT_LT(a, b) ARRAY_SORT_LT(a, b)
#endif
#include "sort/introsort.h"
#ifdef INTROSORT_LT
#undef INTROSORT_LT
#endif
#undef INTROSORT_TYPE
#undef INTROSORT_NAME


static inline bool ARRAY_SORT_FUNC(sort)(ARRAY_NAME *array) {
    if (array == NULL || array->n <= 1) return false;
    ARRAY_SORT_FUNC(introsort)(array->n, array->a);
    return true;
}

static inline bool ARRAY_SORT_FUNC(sort_reverse)(ARRAY_NAME *array) {
    if (array == NULL || array->n <= 1) return false;
    ARRAY_SORT_FUNC(introsort_reverse)(array->n, array->a);
    return true;
}


#define INTROSORT_NAME ARRAY_INDEX_NAME
#define INTROSORT_TYPE size_t
#define INTROSORT_AUX_TYPE ARRAY_TYPE
#include "sort/introsort.h"
#ifdef INTROSORT_LT
#undef INTROSORT_LT
#endif
#undef INTROSORT_NAME
#undef INTROSORT_TYPE
#undef INTROSORT_AUX_TYPE


static inline bool ARRAY_SORT_FUNC(argsort)(ARRAY_NAME *array, size_t *indices) {
    if (array == NULL) return NULL;
    if (indices == NULL) return NULL;
    for (size_t i = 0; i < array->n; i++) {
        indices[i] = i;
    }
    ARRAY_SORT_INDEX_FUNC(introsort)(array->n, indices, array->a);
    return true;
}

static inline bool ARRAY_SORT_FUNC(argsort_reverse)(ARRAY_NAME *array, size_t *indices) {
    if (array == NULL) return false;
    if (indices == NULL) return false;
    for (size_t i = 0; i < array->n; i++) {
        indices[i] = i;
    }
    ARRAY_SORT_INDEX_FUNC(introsort_reverse)(array->n, indices, array->a);
    return true;
}

#undef ARRAY_INDEX_NAME
#undef ARRAY_INDEX_TYPE
#undef ARRAY_SORT_CONCAT
#undef ARRAY_SORT_CONCAT_
#undef ARRAY_SORT_FUNC
#undef ARRAY_SORT_INDEX_FUNC
