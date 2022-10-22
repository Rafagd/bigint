#ifndef BIGINT_H
#define BIGINT_H

#include "array.h"

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

typedef array_t bigint_t;

bigint_t* bigint_new(void);
void      bigint_delete(bigint_t* number);
bool      bigint_resize(bigint_t* number, size_t new_size);

uint32_t  bigint_getbit(bigint_t* number, size_t bitnum);
void      bigint_setbit(bigint_t* number, size_t bitnum, uint32_t value);

bool      bigint_equals(bigint_t* a, bigint_t* b);

bigint_t* bigint_new(void)
{
    return ARRAY_NEW(uint32_t);
}

void bigint_delete(bigint_t* number)
{
    array_delete(number);
}

bool bigint_resize(bigint_t* number, size_t new_size)
{
    if (new_size <= number->size) {
        return true;
    }

    size_t old_size = number->size;

	if (!ARRAY_RESIZE_R(number, new_size)) {
		return false;
	}

    // Right-shifts the whole array new_size-old_size spaces
    memmove(
        array_get(number, new_size - old_size),
        array_get(number, 0),
        old_size * number->item_size
    );
    
    memset(
        array_get(number, 0),
        0,
        (new_size - old_size) * number->item_size
    );
	
    return true;
}

bool bigint_equals(bigint_t* a, bigint_t* b)
{
    size_t max = MAX(a->size, b->size);
    bigint_resize(a, max);
    bigint_resize(b, max);
    return array_equals(a, b);
}

uint32_t bigint_getbit(bigint_t *number, size_t bitnum)
{
    if (bitnum / 32 >= number->size) {
        return 0;
    }

    uint32_t* item = array_get(number, number->size - (bitnum / 32) - 1);
    return *item & (1u << (bitnum % 32));
}

void bigint_setbit(bigint_t *number, size_t bitnum, uint32_t value)
{
    if (bitnum / 32 >= number->size) {
        bigint_resize(number, bitnum / 32);
    }

    value = value != 0 ? 0xFFFFFFFF : 0x00000000;
    value = value & (1u << (bitnum % 32));

    uint32_t* item = array_get(number, number->size - (bitnum / 32) - 1);
    *item |= value;
}

/*

inline
uint32_t bigint_numbits(bigint *b)
{
    //     VecSize   ItemSize           WordSize
    return b->size * sizeof(uint32_t) * 8;
}

void bigint_set(bigint *b, uint32_t *number, int size)
{
    bigint_expand(b, size);
    
    int min_size = MIN(b->size, size);
    for (int i = 0; i < min_size; i++) {
        b->data[b->size - i - 1] = number[size - i - 1];
    }
}

void bigint_set_u32(bigint *b, uint32_t number)
{
    bigint_set(b, &number, 1);
}

uint32_t  bigint_rshift(bigint *b, uint32_t bits)
{
    if (bits == 0) {
        return 0;
    }
    
    int new_data = 0;
    uint32_t carry = 0;
    
    int rmask = 0xFFFFFFFF >> (32 - bits);
    
    for (int i = 0; i < b->size; i++) {
        new_data = carry | (b->data[i] >> bits);
        carry    = (b->data[i] & rmask) << (32 - bits);
        b->data[i] = new_data;
    }
    
    return carry;
}

void bigint_lshift(bigint *b, uint32_t bits)
{
    if (bits == 0) {
        return;
    }
    
    int new_data = 0;
    uint32_t carry = 0;
    
    int lmask = 0xFFFFFFFF << (32 - bits);
    
    for (int i = b->size - 1; i >= 0; i--) {
        new_data = carry | (b->data[i] << bits);
        carry    = (b->data[i] & lmask) >> (32 - bits);
        b->data[i] = new_data;
    }
    
    if (carry != 0) {
        bigint_expand(b, b->size + 1);
        b->data[0] = carry;
    }
}

void bigint_add(bigint *a, bigint *b)
{
    int size = MAX(a->size, b->size);
    bigint_expand(a, size);
    bigint_expand(b, size);
    
    uint32_t low_16  = 0;
    uint32_t high_16 = 0;
    uint32_t carry   = 0;
    for (int i = a->size - 1; i >= 0; i--) {
        low_16 = (a->data[i] & 0x0000FFFF)
               + (b->data[i] & 0x0000FFFF)
               + carry;
        carry = (low_16 & 0xFFFF0000) >> 16;
        
        high_16 = ((a->data[i] & 0xFFFF0000) >> 16)
                + ((b->data[i] & 0xFFFF0000) >> 16)
                + carry;
        carry = (high_16 & 0xFFFF0000) >> 16;
        
        a->data[i] = (high_16 << 16) | (low_16 & 0x0000FFFF);
    }
    
    if (carry != 0) {
        bigint_expand(a, a->size + 1);
        a->data[0] = carry;
    }
}


void bigint_sub(bigint *a, bigint *b)
{
    int size = MAX(a->size, b->size);
    bigint_expand(a, size);
    bigint_expand(b, size);
    
    for (int i = a->size - 1; i >= 0; i--) {
        if (a->data[i] < b->data[i]) {
            if (i == 0) {
                bigint_set_u32(a, 0);
                return;
            }
            a->data[i-1]--;
        }
        a->data[i] -= b->data[i];
    }
}

void bigint_multiply(bigint *a, uint16_t b)
{
    bigint *sum = bigint_new();
    bigint_set_u32(sum, 0);
    
    bigint *pivot = bigint_new();
    bigint_set(pivot, a->data, a->size);

    int last = 0;
    for (int i = 0; i < 16; i++) {
        if (((b >> i) & 0x1) != 0x0) {
            bigint_lshift(pivot, i - last);
            bigint_add(sum, pivot);
            last = i;
        }
    }
    
    bigint_set(a, sum->data, sum->size);
}

void debug(bigint *b)
{
    if (b->size == 0) {
        printf("--------\n");
        return;
    }
    
    for (int i = 0; i < b->size; i++) {
        printf("%08X", b->data[i]);
    }
    printf("\n");
}

int bigint_iszero(bigint *b)
{
    for (int i = 0; i < b->size; i++) {
        if (b->data[i] != 0) {
            return 0;
        }
    }
    return 1;
}

int bigint_msb(bigint *b)
{
    for (int i = 0; i < b->size; i++) {
        if (b->data[i] == 0) {
            continue;
        }
        
        for (int j = 0; j < 32; j++) {
            if (b->data[i] & (0x80000000 >> j)) {
                return (b->size - i - 1) * 32 + 31 - j;
            }
        }
    }
    return -1;
}

int bigint_cmp(bigint *a, bigint *b)
{
    int max_size = MAX(a->size, b->size);
    bigint_expand(a, max_size);
    bigint_expand(b, max_size);
    
    for (int i = 0; i < a->size; i++) {
        if (a->data[i] < b->data[i]) return -1;
        if (a->data[i] > b->data[i]) return  1;
    }
    
    return 0;
}

uint32_t bigint_divide(bigint *a, uint16_t b)
{
    if (b == 0) {
        return 0;
    }
    
    int size = MAX(a->size, 1);
    bigint_expand(a, size);
        
    bigint *d = bigint_new();
    bigint_expand(d, size);
    bigint_set_u32(d, b);
    
    bigint *q = bigint_new();
    bigint_expand(q, size);
    
    bigint *r = bigint_new();
    bigint_expand(r, size);

    for (int i = bigint_numbits(a) - 1; i >= 0; i--) {
        bigint_lshift(r, 1);
        bigint_setbit(r, 0, bigint_getbit(a, i));
        if (bigint_cmp(r, d) >= 0) {
            bigint_sub(r, d);
            bigint_setbit(q, i, 1);
        }
    }
    
    uint32_t remainder = r->size > 0 ? r->data[r->size - 1] : 0;
    bigint_set(a, q->data, q->size);
    bigint_free(q);
    bigint_free(r);
    bigint_free(d);
    return remainder;
}

void bigint_print(bigint *b)
{
    str *s = str_new();
    
    bigint *copy = bigint_new();
    bigint_add(copy, b);
    
    bigint *zero = bigint_new();
    
    int remainder;
    while (bigint_cmp(copy, zero) > 0) {
        remainder = bigint_divide(copy, 10);
        if (remainder < 10) {
            str_push_front(s, '0' + remainder);
        } else {
            str_push_front(s, 'A' + remainder - 10);
        }
    }
    
    str_print(s);
}
    
// Complete the extraLongFactorials function below.
void extraLongFactorials(int n)
{
    bigint *fac = bigint_new();
    bigint_set_u32(fac, n);
    
    for (n -= 1; n > 1; n--) {
        bigint_multiply(fac, n);
    }
    
    bigint_print(fac);
}

int main()
{
    char* n_endptr;
    char* n_str = readline();
    int n = strtol(n_str, &n_endptr, 10);

    if (n_endptr == n_str || *n_endptr != '\0') { exit(EXIT_FAILURE); }

    extraLongFactorials(n);

    return 0;
}

char* readline() {
    size_t alloc_length = 1024;
    size_t data_length = 0;
    char* data = malloc(alloc_length);

    while (true) {
        char* cursor = data + data_length;
        char* line = fgets(cursor, alloc_length - data_length, stdin);

        if (!line) { break; }

        data_length += strlen(cursor);

        if (data_length < alloc_length - 1 || data[data_length - 1] == '\n') { break; }

        size_t new_length = alloc_length << 1;
        data = realloc(data, new_length);

        if (!data) { break; }

        alloc_length = new_length;
    }

    if (data[data_length - 1] == '\n') {
        data[data_length - 1] = '\0';
    }

    data = realloc(data, data_length);

    return data;
}
*/
#endif // BIGINT_H
