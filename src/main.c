#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "utils.h"
#include "parser.h"
#include "dbms_networking.h"
#include "datastructures/database.h"
#include "datastructures/bitset.h"
#include "parser_db_translation.h"

#define PIPE_READ_END 	0
#define PIPE_WRITE_END 	1

int test_function(char msg[MAX_MESSAGE_SIZE], int size, client_id id);
int test_function(char msg[MAX_MESSAGE_SIZE], int size, client_id id)
{
	printf("Message recieved: \"%s\" from %d with size %d\n", msg, id, size);


	

	//dbms_networking_send((char*)(&meme_size), 8, id);
	//dbms_networking_send((char*)(memes), meme_size, id);

	return 1;
}

int main(void)
{
	universe u = create_empty_universe();
	parse_initialization(&u, "./example_db/language_example.txt");

	print_universe(u);

	free_universe(&u);

	return 0;
}

