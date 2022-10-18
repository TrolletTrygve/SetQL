#include <stdio.h>
#include "datastructures/database.h"
#include "utils.h"

int main(void)
{
	Database* db = createEmptyDB(200, 3, 3); 

	char input[] = "coolsetname";
	db_createSet(db, input);

	// add keys
	for (int i = 0; i < 150; i++){
		char dst[12];
		sprintf(dst, "key%d", i);
		db_addKey(db, dst);
	}
	
	char key[] = "key68";

	db_addToSet(db, input, key);

	char attr[] = "attributetable";
	db_createAttribute(db, attr, TYPE_64, -1);

	db_print(db);

	db_removeFromSet(db, input, key);

	db_print(db);

	//db_destroy(db);
	return 0;
}