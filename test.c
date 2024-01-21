#include <stdint.h>
#include "greatest/greatest.h"

#define ARRAY_NAME test_array
#define ARRAY_TYPE int32_t
#define DEFAULT_ARRAY_SIZE 8
#include "array.h"
#undef ARRAY_NAME
#undef ARRAY_TYPE

TEST test_array_resizing(void) {
    test_array *v = test_array_new();
    ASSERT_EQ(v->m, DEFAULT_ARRAY_SIZE);
    ASSERT_EQ(v->n, 0);

    for (int32_t i = 0; i < 10; i++) {
        test_array_push(v, i);
    }
    size_t expected_size = DEFAULT_ARRAY_SIZE * 3 / 2;
    ASSERT_EQ(v->m, expected_size);
    ASSERT_EQ(v->n, 10);

    for (int32_t i = 0; i < 10; i++) {
        ASSERT_EQ(v->a[i], i);
    }

    test_array *w = test_array_new_size(16);
    ASSERT_EQ(w->m, 16);
    ASSERT_EQ(w->n, 0);

    for (int32_t i = 0; i < 17; i++) {
        test_array_push(w, i);
    }
    ASSERT_EQ(w->m, 16 * 3 / 2);
    ASSERT_EQ(w->n, 17);

    test_array_extend(v, w);
    expected_size = expected_size * 3 / 2 * 3 /2;
    ASSERT_EQ(v->m, expected_size);
    ASSERT_EQ(v->n, 27);

    test_array_destroy(v);
    PASS();
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int32_t main(int32_t argc, char **argv) {
    GREATEST_MAIN_BEGIN();      /* command-line options, initialization. */

    RUN_TEST(test_array_resizing);

    GREATEST_MAIN_END();        /* display results */
}
