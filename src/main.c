#include <stdio.h>
#include "dbms_networking.h"

int main(void)
{
	int result;
	printf("Hello world!\n");

	if (!dbms_networking_initialize(8080))
	{
		printf("fan");
	}

	result = dbms_start();
	if (!result)
	{	
		fprintf(stderr, "fan");
	}

	result = dbms_networking_kill();

	return 0;
}