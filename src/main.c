#include <stdio.h>
#include "dbms_networking.h"

int main(void)
{
	
	printf("Hello world!\n");

	if (!dbms_networking_initialize(8080))
	{
		printf("fan");
	}

	return 0;
}