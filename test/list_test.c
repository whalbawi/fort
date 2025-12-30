#include "list.h"

#include <stddef.h>  // for NULL

#include "test.h"    // for TEST_ASSERT_NONNULL, TEST, TEST_RUN, TEST_ASSERT...

TEST(create_list, {
    list_t list;
    list_init(&list);
    list_fini(&list);
})

TEST(empty_list_pop, {
    list_t list;
    list_init(&list);

    void* data = list_pop(&list);
    TEST_ASSERT_TRUE(data == NULL);

    list_fini(&list);
})

TEST(push_single_item, {
    list_t list;
    list_init(&list);

    int value = 42;
    list_push(&list, &value);

    int* result = (int*)list_pop(&list);
    TEST_ASSERT_NONNULL(result);
    TEST_ASSERT_EQ_INT32(*result, 42);

    list_fini(&list);
})

TEST(push_multiple_items, {
    list_t list;
    list_init(&list);

    int value1 = 10;
    int value2 = 20;
    int value3 = 30;

    list_push(&list, &value1);
    list_push(&list, &value2);
    list_push(&list, &value3);

    int* result3 = (int*)list_pop(&list);
    TEST_ASSERT_NONNULL(result3);
    TEST_ASSERT_EQ_INT32(*result3, 30);

    int* result2 = (int*)list_pop(&list);
    TEST_ASSERT_NONNULL(result2);
    TEST_ASSERT_EQ_INT32(*result2, 20);

    int* result1 = (int*)list_pop(&list);
    TEST_ASSERT_NONNULL(result1);
    TEST_ASSERT_EQ_INT32(*result1, 10);

    void* empty = list_pop(&list);
    TEST_ASSERT_TRUE(empty == NULL);

    list_fini(&list);
})

TEST(push_pop_interleaved, {
    list_t list;
    list_init(&list);

    int value1 = 100;
    int value2 = 200;

    list_push(&list, &value1);
    int* result1 = (int*)list_pop(&list);
    TEST_ASSERT_NONNULL(result1);
    TEST_ASSERT_EQ_INT32(*result1, 100);

    list_push(&list, &value2);
    int* result2 = (int*)list_pop(&list);
    TEST_ASSERT_NONNULL(result2);
    TEST_ASSERT_EQ_INT32(*result2, 200);

    void* empty = list_pop(&list);
    TEST_ASSERT_TRUE(empty == NULL);

    list_fini(&list);
})

TEST(push_null_data, {
    list_t list;
    list_init(&list);

    list_push(&list, NULL);

    void* result = list_pop(&list);
    TEST_ASSERT_TRUE(result == NULL);

    list_fini(&list);
})

TEST(large_number_of_items, {
    list_t list;
    list_init(&list);

    int values[100];
    for (int i = 0; i < 100; i++) {
        values[i] = i;
        list_push(&list, &values[i]);
    }

    for (int i = 99; i >= 0; i--) {
        int* result = (int*)list_pop(&list);
        TEST_ASSERT_NONNULL(result);
        TEST_ASSERT_EQ_INT32(*result, i);
    }

    void* empty = list_pop(&list);
    TEST_ASSERT_TRUE(empty == NULL);

    list_fini(&list);
})

TEST(fini_with_items, {
    list_t list;
    list_init(&list);

    int value1 = 1;
    int value2 = 2;
    int value3 = 3;

    list_push(&list, &value1);
    list_push(&list, &value2);
    list_push(&list, &value3);

    list_fini(&list);
})

TEST(fini_null_list, { list_fini(NULL); })

TEST(different_data_types, {
    list_t list;
    list_init(&list);

    int int_val = 42;
    const double pi_approx = 3.14;
    double double_val = pi_approx;
    char char_val = 'A';

    list_push(&list, &int_val);
    list_push(&list, &double_val);
    list_push(&list, &char_val);

    char* char_result = (char*)list_pop(&list);
    TEST_ASSERT_NONNULL(char_result);
    TEST_ASSERT_EQ_CHAR(*char_result, 'A');

    double* double_result = (double*)list_pop(&list);
    TEST_ASSERT_NONNULL(double_result);

    int* int_result = (int*)list_pop(&list);
    TEST_ASSERT_NONNULL(int_result);
    TEST_ASSERT_EQ_INT32(*int_result, 42);

    list_fini(&list);
})

TEST(push_after_pop_all, {
    list_t list;
    list_init(&list);

    int value1 = 10;
    list_push(&list, &value1);

    int* result1 = (int*)list_pop(&list);
    TEST_ASSERT_NONNULL(result1);
    TEST_ASSERT_EQ_INT32(*result1, 10);

    void* empty = list_pop(&list);
    TEST_ASSERT_TRUE(empty == NULL);

    int value2 = 20;
    list_push(&list, &value2);

    int* result2 = (int*)list_pop(&list);
    TEST_ASSERT_NONNULL(result2);
    TEST_ASSERT_EQ_INT32(*result2, 20);

    list_fini(&list);
})

int main(int argc, char* argv[]) {
    TEST_INIT("list", argc, argv);

    TEST_RUN(create_list);
    TEST_RUN(empty_list_pop);
    TEST_RUN(push_single_item);
    TEST_RUN(push_multiple_items);
    TEST_RUN(push_pop_interleaved);
    TEST_RUN(push_null_data);
    TEST_RUN(large_number_of_items);
    TEST_RUN(fini_with_items);
    TEST_RUN(fini_null_list);
    TEST_RUN(different_data_types);
    TEST_RUN(push_after_pop_all);

    TEST_EXIT();
}
