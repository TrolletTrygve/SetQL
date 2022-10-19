#include <stdio.h>
#include "datastructures/database.h"
#include "utils.h"
#include "datastructures/bitset.h"

void bitset_test(void);


int main(void)
{

	//bitset_test();

	
	Database* db = createEmptyDB(200, 3, 3); 

	char input[] = "coolsetname";
	db_createSet(db, input);

	// add keys
	for (int i = 0; i < 150; i++){
		char dst[12];
		sprintf(dst, "key%d", i);
		db_addKey(db, dst);
	}
	
	char key[] = "key110";

	db_addToSet(db, input, key);

	char attr[] = "attributetable";
	db_createAttribute(db, attr, TYPE_8, -1);

	db_print(db);

	db_removeFromSet(db, input, key);

	printf("whaaat!\n");
	db_print(db);
   	
   	fprintf(stderr,"asdasd\n");
	db_destroy(db);
	
	return 0;
}


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