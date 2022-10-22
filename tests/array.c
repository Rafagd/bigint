#include<stdlib.h>
#include<stdbool.h>
#include<check.h>

#include<time.h>
#include<array.h>
#include<stdio.h>

Suite* array_suite(void);

START_TEST(test_array_create_and_delete)
{
    array_t* array;
   
    array = array_new(sizeof(uint32_t));
    ck_assert(array            != NULL);             // Could not allocate memory
    ck_assert(array->items     == NULL);             // Should be initialized to NULL
    ck_assert(array->item_size == sizeof(uint32_t)); // Should contain the same size provided
    ck_assert(array->size      == 0);                // Should start at 0
    ck_assert(array->capacity  == 0);                // Should start at 0
    // Can't really test this, but if it somehow crashes it will fail the test
    array_delete(array);                             // Should not crash

    // Same tests, but using the convenience macro
    array = ARRAY_NEW(uint32_t);
    ck_assert(array            != NULL);
    ck_assert(array->items     == NULL);
    ck_assert(array->item_size == sizeof(uint32_t));
    ck_assert(array->size      == 0);
    ck_assert(array->capacity  == 0);
    array_delete(array);
}
END_TEST

START_TEST(test_array_resize)
{
    array_t* array = ARRAY_NEW(uint32_t); // <--
    ck_assert(array != NULL); // Should be able to allocate memory

    // Storing old pointer for reallocation detection
    uint8_t* old_items = array->items;

    uint8_t zeroes[sizeof(uint32_t) * 10];
    memset(&zeroes, 0, sizeof(uint32_t) * 10);

    bool     res;
    uint32_t one = 1;

    // Adds an item, should reallocate
    res = ARRAY_RESIZE(array, 1);
    ck_assert(res);
    ck_assert(array->items     != NULL);      // Items should point to something
    ck_assert(array->items     != old_items); // And should point to new memory
    ck_assert(memcmp(array->items, &zeroes, sizeof(uint32_t) * 1) == 0); // Also check if it's initialized correctly
    ck_assert(array->item_size == sizeof(uint32_t)); // This should not change
    ck_assert(array->size      == 1);         // Size should be updated to 1
    ck_assert(array->capacity  == 1);         // Capacity should be updated to 1
    old_items = array->items;                 // update old pointer

    // We will check if realloc is changing valid items
    memcpy(zeroes,       &one, sizeof(uint32_t));
    memcpy(array->items, &one, sizeof(uint32_t));

    // Add 10 items, should reallocate again
    res = ARRAY_RESIZE(array, 10);           
    ck_assert(res);
    ck_assert(array->items     != NULL);
    ck_assert(array->items     != old_items); 
    ck_assert(memcmp(array->items, zeroes, 10 * sizeof(uint32_t)) == 0); // Check if only new values are 0
    ck_assert(array->item_size == sizeof(uint32_t));
    ck_assert(array->size      == 10);
    ck_assert(array->capacity  == 10);
    old_items = array->items;

    // Remove 5 items, should not reallocate
    res = ARRAY_RESIZE(array, 5);
    ck_assert(res);
    ck_assert(array->items     != NULL);       // Same as before
    ck_assert(array->items     == old_items);  // But should point to the same memory
    ck_assert(memcmp(array->items, zeroes, 5 * sizeof(uint32_t)) == 0);
    ck_assert(array->item_size == sizeof(uint32_t)); 
    ck_assert(array->size      == 5);          // And update the size
    ck_assert(array->capacity  == 10);         // But not the capacity
    old_items = array->items;

    // We're changing values past the boundary to check for reinitialization
    memcpy(array->items + 6 * sizeof(uint32_t), &one, sizeof(uint32_t));

    // Add 2 items, should not reallocate
    res = ARRAY_RESIZE(array, 7); // <--
    ck_assert(res);
    ck_assert(array->items     != NULL);       // Same as before
    ck_assert(array->items     == old_items);  // But we're adding new positions again
    ck_assert(memcmp(array->items, zeroes, 7 * sizeof(uint32_t)) == 0); // items[6] should be 0
    ck_assert(array->item_size == sizeof(uint32_t));
    ck_assert(array->size      == 7);
    ck_assert(array->capacity  == 10);

    array_delete(array);
}
END_TEST

START_TEST(test_array_getter_and_setter)
{
    uint32_t values[10];

    array_t* array = ARRAY_NEW(uint32_t);
    bool res = ARRAY_RESIZE(array, 10);
    ck_assert(res);                  // Assert if successful
    ck_assert(array->items != NULL); // Should have been resized to 10.

    // Set a regular array and this array to a random number
    for (size_t i = 0; i < 10; i++) {
        uint32_t r = (uint32_t) rand();
        values[i] = r;
        array_set(array, i, &r);
    }

    // Compare the values of set manually with the ones set using the function
    ck_assert(memcmp(array->items, values, 10 * sizeof(uint32_t)) == 0);

    // Check if the get function is working properly
    for (size_t i = 0; i < 10; i++) {
        ck_assert(*((uint32_t*) array_get(array, i)) == values[i]);
    }

    array_delete(array);
}
END_TEST

START_TEST(test_array_equals)
{
    array_t* a = ARRAY_NEW(uint32_t);
    array_t* b = ARRAY_NEW(uint32_t);
    array_t* c = ARRAY_NEW(uint16_t);

    ck_assert( array_equals(a, b)); // Should be equal
    ck_assert(!array_equals(a, c)); // Should be not equal (different type sizes)

    ARRAY_RESIZE(a, 10);
    ARRAY_RESIZE(b, 10);
    ARRAY_RESIZE(c, 10);

    ck_assert( array_equals(a, b)); // Should be equal
    ck_assert(!array_equals(a, c)); // Should be not equal (different type sizes)
    
    uint32_t u32_tmp = 0x00000001;
    uint32_t u16_tmp = 0x0001;
    array_set(a, a->size-1, &u32_tmp);
    array_set(b, b->size-1, &u32_tmp);
    array_set(c, c->size-1, &u16_tmp);

    ck_assert( array_equals(a, b)); // Should be equal
    ck_assert(!array_equals(a, c)); // Should be not equal (different type sizes)

    u32_tmp = 0x55555555;
    u16_tmp = 0x5555;
    for (size_t i = 0; i < 10; i++) {
        array_set(a, i, &u32_tmp);
        array_set(b, i, &u32_tmp);
        array_set(c, i, &u16_tmp);
    }

    ck_assert( array_equals(a, b)); // Should be equal
    ck_assert(!array_equals(a, c)); // Should be not equal (different type sizes)

    u32_tmp = 0xAAAAAAAA;
    u16_tmp = 0xAAAA;
    for (size_t i = 0; i < 10; i++) {
        array_set(b, i, &u32_tmp);
        array_set(c, i, &u16_tmp);
    }

    ck_assert(!array_equals(a, b)); // Should be not equal (different values)
    ck_assert(!array_equals(a, c)); // Should be not equal (different type sizes)

    array_delete(a);
    array_delete(b);
    array_delete(c);
}
END_TEST

START_TEST(test_array_push_and_pop_back)
{
    uint32_t values[10];

    array_t* array = ARRAY_NEW(uint32_t);

    // Set a regular array and this array to a random number
    for (int i = 0; i < 10; i++) {
        uint32_t r = (uint32_t) rand();
        values[i] = r;
        ck_assert(array_push_back(array, &r)); // returns false if not allocated
    }

    // Compare the values of set manually with the ones set using the function
    ck_assert(memcmp(array->items, values, 10 * sizeof(uint32_t)) == 0);
    ck_assert(array->size     == 10); // Check if we have 10 elements
    ck_assert(array->capacity == 10); // And the capacity is 10

    // Set a regular array and this array to a random number
    for (int i = 9; i >= 0; i--) {
        uint32_t* item = array_pop_back(array);
        ck_assert( item != NULL);
        ck_assert(*item == values[i]);
        free(item);
    }

    ck_assert(array->size     == 0);  // Check if we have 0 elements
    ck_assert(array->capacity == 10); // And the capacity is 10

    uint8_t* old_items = array->items;

    // Add 5 back
    for (int i = 0; i < 5; i++) {
        uint32_t r = (uint32_t) rand();
        values[i] = r;
        ck_assert(array_push_back(array, &r));
    }

    ck_assert(array->items    == old_items); // Should not have reallocated
    ck_assert(array->size     == 5);  // Check if we have 0 elements
    ck_assert(array->capacity == 10); // And the capacity is 10

    array_delete(array);
}
END_TEST

START_TEST(test_array_push_and_pop_front)
{
    uint32_t values[10];

    array_t* array = ARRAY_NEW(uint32_t);

    // Set a regular array and this array to a random number
    for (int i = 0; i < 10; i++) {
        uint32_t r = (uint32_t) rand();
        values[9 - i] = r; // Adding in reverse to make memcmp easier
        ck_assert(array_push_front(array, &r));
    }

    // Compare the values of set manually with the ones set using the function
    ck_assert(memcmp(array->items, values, 10 * sizeof(uint32_t)) == 0);
    ck_assert(array->size     == 10); // Check if we have 10 elements
    ck_assert(array->capacity == 10); // And the capacity is 10

    // Set a regular array and this array to a random number
    for (int i = 0; i < 10; i++) {
        uint32_t* item = array_pop_front(array);
        ck_assert( item != NULL);
        ck_assert(*item == values[i]);
        free(item);
    }

    ck_assert(array->size     == 0);  // Check if we have 0 elements
    ck_assert(array->capacity == 10); // And the capacity is 10

    uint8_t* old_items = array->items;

    // Add 5 to front
    for (int i = 0; i < 5; i++) {
        uint32_t r = (uint32_t) rand();
        values[i] = r;
        ck_assert(array_push_front(array, &r));
    }

    ck_assert(array->items    == old_items); // Should not have reallocated
    ck_assert(array->size     == 5);  // Check if we have 0 elements
    ck_assert(array->capacity == 10); // And the capacity is 10

    array_delete(array);
}
END_TEST

Suite* array_suite(void)
{
    Suite* s;
    TCase* tc_core;

    s = suite_create("Array");

    tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_array_create_and_delete);
    tcase_add_test(tc_core, test_array_resize);
    tcase_add_test(tc_core, test_array_getter_and_setter);
    tcase_add_test(tc_core, test_array_equals);
    tcase_add_test(tc_core, test_array_push_and_pop_back);
    tcase_add_test(tc_core, test_array_push_and_pop_front);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(int argc, char** argv)
{
    srand((unsigned int) time(NULL));

    Suite*   s  = array_suite();
    SRunner* sr = srunner_create(s);

    // TODO: Remove if not debugging!
    srunner_set_fork_status(sr, CK_NOFORK);

    srunner_run_all(sr, CK_VERBOSE);
    int failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
