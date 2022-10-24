#include "parser.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "dbms_networking.h"
#include "datastructures/database.h"
#include "utils.h"
#include "datastructures/bitset.h"

#define PIPE_READ_END 	0
#define PIPE_WRITE_END 	1

int test_function(char msg[MAX_MESSAGE_SIZE], int size, client_id id);
int test_function(char msg[MAX_MESSAGE_SIZE], int size, client_id id)
{
	printf("Message recieved: \"%s\" from %d with size %d\n", msg, id, size);
	return 1;
}

int main(void)
{
	//universe u = create_universe_example();
	//print_universe(u);

	universe u = create_empty_universe();
	parse_initialization(&u, "./initialization_example.txt");

	print_universe(u);

	free_universe(&u);

	query q = create_empty_query();
	parse_query(&q, "SELECT name, population FROM COMPLEMENT (COMPLEMENT CanFly);");

	return 0;
}

