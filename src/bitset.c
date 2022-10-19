#include "datastructures/bitset.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/**
 * @brief Allocates and initializes a bitset
 * 
 * @param size The amount of bits the bitset will hold
 * @param size The amount of bits the bitset will hold
 * 
 * @return The newly created int pointer (bitset)
 */
bitset* bitset_create(int size, int default_value)
{
	// figure out alloc size
	int integer_count = ceil((float)size / ((float)INTEGER_BIT_SIZE));
	int alloc_size = sizeof(uint64_t) * integer_count;

	// allocate the memory and return NULL on error
	bitset* b = (bitset*)malloc(sizeof(bitset));
	if (b == NULL)
	{
		perror("malloc");
		return NULL;
	}

	b->bits = (uint64_t*)malloc(alloc_size);
	if (b->bits == NULL)
	{
		perror("malloc");
		return NULL;
	}		

	// set the memory to the default value
	memset(b->bits, default_value, alloc_size);
	b->integer_count = (uint64_t)integer_count;

	return b;
}


/**
 * @brief      Creates a set without initializing the defualt values
 *
 * @param[in]  size  The number of bits in the set
 *
 * @return     A pointer to the newly created bitset
 */
bitset* bitset_create_no_default(int size)
{
	// figure out alloc size
	int integer_count = ceil((float)size / ((float)INTEGER_BIT_SIZE));
	int alloc_size = sizeof(uint64_t) * integer_count;

	// allocate the memory and return NULL on error
	bitset* b = (bitset*)malloc(sizeof(bitset));
	if (b == NULL)
	{
		perror("malloc");
		return NULL;
	}

	b->bits = (uint64_t*)malloc(alloc_size);
	if (b->bits == NULL)
	{
		perror("malloc");
		return NULL;
	}

	b->integer_count = (uint64_t)integer_count;
	return b;
}


/**
 * @brief      Initializes a bitset by allocating memory for the bits and
 *				setting the size 	
 *	
 * @param      b              The bitset to initialize
 * @param[in]  size           The number of bits in the set
 * @param[in]  default_value  The default value of the bits
 */
void bitset_initialize(bitset* b, int size, int default_value)
{
	// figure out alloc size
	int integer_count = ceil((float)size / ((float)INTEGER_BIT_SIZE));
	int alloc_size = sizeof(uint64_t) * integer_count;

	b->bits = (uint64_t*)malloc(alloc_size);
	if (b->bits == NULL)
	{
		perror("malloc");
		return;
	}

	memset(b->bits, default_value, alloc_size);
	b->integer_count = (uint64_t)integer_count;
}


/**
 * @brief      Creates the complement set to b (needs to be freed)
 *
 * @param      b     The bitset to create the complement to
 *
 * @return     The complement bitset
 */
bitset* bitset_complement(bitset* b)
{
	// Create a new bitset of the same size as b
	bitset* c = bitset_create_no_default(b->integer_count * INTEGER_BIT_SIZE);

	for (uint64_t i = 0; i < b->integer_count; i++)
		c->bits[i] = ~(b->bits[i]);

	return c;
}


/**
 * @brief      Creates the intersection of b1 and b2 (needs to be freed)
 *
 * @param      b1    The left hand side in (A intersect B)
 * @param      b2    The right hand side (A intersect B)
 *
 * @return     A new bitset containing the intersection of b1 and b2
 */
bitset* bitset_intersection(bitset* b1, bitset* b2)
{
	uint64_t size = b1->integer_count;
	if (b1->integer_count > b2->integer_count)
		size = b2->integer_count;

	bitset* intersection = bitset_create_no_default(size * INTEGER_BIT_SIZE);

	for (uint64_t i = 0; i < size; i++)
		intersection->bits[i] = b1->bits[i] & b2->bits[i];	

	return intersection;
}


/**
 * @brief      Creates the union of b1 and b2 (needs to be freed)
 *
 * @param      b1    The left hand side in (A union B)
 * @param      b2    The right hand side in (A union B)
 *
 * @return     A new bitset containing the union of b1 and b2
 */
bitset* bitset_union(bitset* b1, bitset* b2)
{
	uint64_t size = b1->integer_count;
	if (b1->integer_count > b2->integer_count)
		size = b2->integer_count;

	bitset* intersection = bitset_create_no_default(size * INTEGER_BIT_SIZE);

	for (uint64_t i = 0; i < size; i++)
		intersection->bits[i] = b1->bits[i] | b2->bits[i];	

	return intersection;
}


/**
 * @brief      Creates the difference between b1 and b2 (needs to be freed)
 *
 * @param      b1    The left hand side in (A / B)
 * @param      b2    The right hand side in (A / B)
 *
 * @return     A new bitset containing the difference between b1 and b2
 */
bitset* bitset_difference(bitset* b1, bitset* b2)
{
	uint64_t size = b1->integer_count;
	if (b1->integer_count > b2->integer_count)
		size = b2->integer_count;

	bitset* diff = bitset_create_no_default(size * INTEGER_BIT_SIZE);

	for (uint64_t i = 0; i < size; i++)
		diff->bits[i] = b1->bits[i] & ~(b2->bits[i]);	

	return diff;
}

/**
 * @brief      Creates the symmetric difference between b1 and b2			
 * 				(needs to be freed)			
 *	 			
 * @param      b1    The left hand side in (A symmetric difference B)
 * @param      b2    The right hand side in (A symmetric difference B)
 *
 * @return     A new bitset containing the symmetric difference
 *  			between b1 and b2
 */
bitset* bitset_symmetric_difference(bitset* b1, bitset* b2)
{
	uint64_t size = b1->integer_count;
	if (b1->integer_count > b2->integer_count)
		size = b2->integer_count;

	bitset* diff = bitset_create_no_default(size * INTEGER_BIT_SIZE);

	for (uint64_t i = 0; i < size; i++)
		diff->bits[i] = b1->bits[i] ^ (b2->bits[i]);	

	return diff;
}


/**
 * @brief Sets a specific bit in the bitset
 * 
 * @param b The bitset to set a bit in
 * @param bit The bit to set
 */
inline void bitset_set_bit(bitset* b, uint64_t bit)
{
	uint64_t i = bit / INTEGER_BIT_SIZE;
	uint64_t bit_placement = bit % INTEGER_BIT_SIZE;

	b->bits[i] = b->bits[i] | ((uint64_t)1 << bit_placement);
}


/**
 * @brief Clears a specific bit in the bitset
 * 
 * @param b The bitset to clear a bit in
 * @param bit The bit to clear
 */
inline void bitset_clear_bit(bitset* b, uint64_t bit)
{
	uint64_t i = bit / INTEGER_BIT_SIZE;
	uint64_t bit_placement = bit % INTEGER_BIT_SIZE;

	b->bits[i] = b->bits[i] & ~((uint64_t)1 << bit_placement);
}

/**
 * @brief Prints the individual bits of a bitset to stdout
 * 
 * @param b The bitset to print
 */
void bitset_print(bitset* b)
{
	printf("[");
	for (uint64_t i = 0; i < b->integer_count; i++)
	{
		uint64_t byte = b->bits[i];
		for (int j = 0; j < INTEGER_BIT_SIZE; j++)
		{
			uint64_t bit = (byte >> j) & 1;
			printf("%lu", bit);
		}
	}
	printf("]\n");
}


/**
 * @brief Frees the memory allocated to the bitset
 * 
 * @param b The bitset to free the memory of
 */
void bitset_free(bitset* b)
{
	free(b->bits);
	free(b);
}


/**
 * @brief      Tests the bitset
 */
void bitset_test(void)
{
	bitset* b1 = bitset_create(128, BIT_CLEAR);
	bitset* b2 = bitset_create(128, BIT_CLEAR);

	bitset_set_bit(b1, 3);
	bitset_set_bit(b2, 3);

	bitset_set_bit(b1, 0);
	bitset_set_bit(b1, 2);

	bitset_set_bit(b2, 5);
	bitset_set_bit(b2, 120);

	bitset* b3 = bitset_intersection(b1, b2);
	bitset* b4 = bitset_union(b1, b2);
	bitset* b5 = bitset_difference(b1, b2);
	bitset* b6 = bitset_difference(b2, b1);
	bitset* b7 = bitset_symmetric_difference(b1, b2);

	bitset_print(b1);
	bitset_print(b2);
	bitset_print(b3);
	bitset_print(b4);
	bitset_print(b5);
	bitset_print(b6);
	bitset_print(b7);

	bitset_free(b1);
	bitset_free(b2);
	bitset_free(b3);
	bitset_free(b4);
	bitset_free(b5);
	bitset_free(b6);
	bitset_free(b7);
}