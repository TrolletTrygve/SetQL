#ifndef __BITSET_H__
#define __BITSET_H__

#include <stdint.h>

#define BIS_SET 	0xFF	
#define BIT_CLEAR 	0

#define INTEGER_BIT_SIZE 64

typedef struct 
{
	uint64_t integer_count;
	uint64_t* bits;
} bitset;

// Function delcarations
bitset* bitset_create(int size, int default_value);
bitset* bitset_create_no_default(int size);

void bitset_initialize(bitset* b, int size, int default_value);

bitset* bitset_complement(bitset* b);
bitset* bitset_intersection(bitset* b1, bitset* b2);
bitset* bitset_union(bitset* b1, bitset* b2);
bitset* bitset_difference(bitset* b1, bitset* b2);
bitset* bitset_symmetric_difference(bitset* b1, bitset* b2);

void bitset_set_bit(bitset* b, uint64_t bit);
void bitset_clear_bit(bitset* b, uint64_t bit);

void bitset_print(bitset* b);
void bitset_free(bitset* b);

#endif