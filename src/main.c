#include <stdio.h>
#include "datastructures/database.h"

int main(void)
{
	printf("Hello world!\n");
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

	db_print(db);

	db_destroy(db);
	return 0;
}