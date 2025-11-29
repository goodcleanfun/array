#include <stdint.h>
#include "greatest/greatest.h"

#define ARRAY_NAME test_array
#define ARRAY_TYPE int32_t
#define DEFAULT_ARRAY_SIZE 8
#include "array.h"
#include "sort.h"
#undef ARRAY_NAME
#undef ARRAY_TYPE

TEST test_dynamic_array(void) {
    test_array *v = test_array_new();
    ASSERT_EQ(test_array_capacity(v), DEFAULT_ARRAY_SIZE);
    ASSERT(test_array_empty(v));
    ASSERT_EQ(test_array_size(v), 0);

    for (int32_t i = 0; i < 10; i++) {
        test_array_push(v, i);
    }
    size_t expected_size = DEFAULT_ARRAY_SIZE * 3 / 2;
    ASSERT_EQ(test_array_capacity(v), expected_size);
    ASSERT(!test_array_empty(v));
    ASSERT_EQ(test_array_size(v), 10);

    for (size_t i = 0; i < 10; i++) {
        test_array_set(v, i, 10 - i);
    }

    int32_t test_value;
    int32_t *test_value_ptr;
    for (int32_t i = 0; i < 10; i++) {
        ASSERT_EQ(test_array_get_unchecked(v, i), 10 - i);
        ASSERT(test_array_get(v, i, &test_value));
        ASSERT_EQ(test_value, 10 - i);
        ASSERT(test_array_at(v, i, &test_value_ptr));
        ASSERT_EQ(test_value_ptr, v->a + i);
        ASSERT_EQ(*test_value_ptr, 10 - i);
    }

    for (int32_t i = 0; i < 10; i++) {
        test_array_set_unchecked(v, i, i);
    }

    test_array *w = test_array_new_size(16);
    ASSERT_EQ(test_array_capacity(w), 16);
    ASSERT_EQ(test_array_size(w), 0);

    for (int32_t i = 0; i < 17; i++) {
        test_array_push(w, i);
    }
    ASSERT_EQ(test_array_capacity(w), 16 * 3 / 2);
    ASSERT_EQ(test_array_size(w), 17);

    test_array_concat(v, w);
    expected_size = expected_size * 3 / 2 * 3 / 2;
    ASSERT_EQ(test_array_capacity(v), expected_size);
    ASSERT_EQ(test_array_size(v), 27);
    size_t current_cap = v->m;
    ASSERT(test_array_resize_to_fit(v, current_cap));
    ASSERT_EQ(test_array_capacity(v), current_cap);

    test_array_extend(v, (int32_t[]){1, 2, 3}, 3);
    expected_size = expected_size * 3 / 2;
    ASSERT_EQ(test_array_capacity(v), expected_size);
    ASSERT_EQ(test_array_size(v), 30);
    current_cap = test_array_capacity(v);
    ASSERT(test_array_resize_to_fit(v, (current_cap * 3 / 2) - 1));
    expected_size = expected_size * 3 / 2;
    ASSERT_EQ(test_array_capacity(v), expected_size);

    test_array_destroy(v);
    PASS();
}

TEST test_array_sorting(void) {
    test_array *v = test_array_new();

    // Test empty array
    test_array_sort(v);
    ASSERT(test_array_empty(v));

    // Test single element
    test_array_push(v, 1);
    test_array_sort(v);
    ASSERT_EQ(test_array_get_unchecked(v, 0), 1);

    // Test multiple elements
    test_array_clear(v);
    int32_t values[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    test_array_extend(v, values, 9);

    // Test sort
    test_array_sort(v);
    for (size_t i = 0; i < test_array_size(v) - 1; i++) {
        ASSERT(test_array_get_unchecked(v, i) <= test_array_get_unchecked(v, i + 1));
    }

    // Test sort_reverse
    test_array_sort_reverse(v);
    for (size_t i = 0; i < test_array_size(v) - 1; i++) {
        ASSERT(test_array_get_unchecked(v, i) >= test_array_get_unchecked(v, i + 1));
    }

    // Test argsort
    test_array_clear(v);
    test_array_extend(v, values, 9);
    size_t *indices = malloc(test_array_size(v) * sizeof(size_t));
    ASSERT(test_array_argsort(v, indices));
    for (size_t i = 0; i < test_array_size(v) - 1; i++) {
        size_t idx1 = indices[i];
        size_t idx2 = indices[i + 1];
        ASSERT(test_array_get_unchecked(v, idx1) <= test_array_get_unchecked(v, idx2));
    }
    // Test argsort_reverse
    test_array_clear(v);
    test_array_extend(v, values, 9);
    ASSERT(test_array_argsort_reverse(v, indices));
    for (size_t i = 0; i < test_array_size(v) - 1; i++) {
        size_t idx1 = indices[i];
        size_t idx2 = indices[i + 1];
        ASSERT(test_array_get_unchecked(v, idx1) >= test_array_get_unchecked(v, idx2));
    }
    free(indices);

    test_array_destroy(v);
    PASS();
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int32_t main(int32_t argc, char **argv) {
    GREATEST_MAIN_BEGIN();      /* command-line options, initialization. */

    RUN_TEST(test_dynamic_array);
    RUN_TEST(test_array_sorting);

    GREATEST_MAIN_END();        /* display results */
}
