#include<stdlib.h>
#include<stdbool.h>
#include<check.h>

#include<time.h>
#include<bigint.h>
#include<stdio.h>

Suite* bigint_suite(void);

START_TEST(test_bigint_create_and_delete)
{
    bigint_t* number = bigint_new();
    ck_assert(number        != NULL); // Successful allocation
    ck_assert(number->items == NULL); // Successful allocation
    bigint_delete(number);
}
END_TEST

START_TEST(test_bigint_resize)
{
    bigint_t* number = bigint_new();
    ck_assert(number        != NULL); // Successful allocation
    ck_assert(number->items == NULL); // Successful allocation

    ck_assert(bigint_resize(number, 5));
    ck_assert(number->items != NULL); // Should not be null
    ck_assert(number->size  == 5);    // Should have the size suggested
    
    uint32_t one = 1;
    array_set(number, 4, &one);

    uint8_t* old_ptr = number->items; 
    ck_assert(bigint_resize(number, 10));
    ck_assert(number->items != NULL);    // Should not be null
    ck_assert(number->items != old_ptr); // Should have reallocated
    ck_assert(number->size  == 10);      // Should have the size suggested

    for (size_t i = 0; i < 10; i++) {
        uint32_t* item = array_get(number, i);
        ck_assert(*item == (i == 9)); // 0 everywhere except number->items[9]
    }
    
    old_ptr = number->items; 
    ck_assert(bigint_resize(number, 2));
    ck_assert(number->items != NULL);    // Should not be null
    ck_assert(number->items == old_ptr); // Should not have reallocated
    ck_assert(number->size  == 10);      // Should maintain old size
    ck_assert(*((uint32_t*) array_get(number, 9)) == 1); // 1 should have stayed here

    for (size_t i = 0; i < 10; i++) {
        uint32_t* item = array_get(number, i);
        ck_assert(*item == (i == 9)); // Value should be unchanged.
    }

    bigint_delete(number);
}
END_TEST

Suite* bigint_suite(void)
{
    Suite* s;
    TCase* tc_core;

    s = suite_create("BigInt");

    tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_bigint_create_and_delete);
    tcase_add_test(tc_core, test_bigint_resize);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(int argc, char** argv)
{
    srand((unsigned int) time(NULL));

    Suite*   s  = bigint_suite();
    SRunner* sr = srunner_create(s);

    // TODO: Remove if not debugging!
    srunner_set_fork_status(sr, CK_NOFORK);

    srunner_run_all(sr, CK_VERBOSE);
    int failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
