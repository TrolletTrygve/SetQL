#include <stdio.h>
#include "datastructures/database.h"

int main(void)
{
	printf("Hello world!\n");
	char elem[][50] = {
		"hye",
		"hej"
	};
	Database* db = createEmptyDB(100, 3, 3, elem); 

	char input[] = "meeeme";
	addEmptySet(db, input);

	printDB(db);

	destroyDB(db);
	return 0;
}