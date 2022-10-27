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

static Database* db;
static universe u;

int test_function(char msg[MAX_MESSAGE_SIZE], int size, client_id id);
int test_function(char msg[MAX_MESSAGE_SIZE], int size, client_id id)
{
	printf("Message recieved: \"%s\" from %d with size %d\n", msg, id, size);

	query q = create_empty_query();
	int error = parse_query(&q, msg);
	if (error)
	{
		fprintf(stderr, "%s\n", "Error, parse_query");
		return -1;
	}

	// Debug prints
	#ifdef DEBUG
		/*
		uint64_t data_length = ret->dataLength;
		uint64_t col_count = ret->columnCount;

		void* col_data[64];
		int col_type[64];
		for (int i = 0; i < col_count; i++)
		{
			col_data[i] = (void*)ret->columns[i].data;
		}
		*/
	#endif

	//QueryReturn* ret = db_run_query(db, &q);

	// Send amount of columns
	// 
	// Send length of data
	// 
	// For each column
	// 	send type for column
	// 	send data size of column
	//	send data for column

	free_query(&q);

	//dbms_networking_send((char*)(&meme_size), 8, id);
	//dbms_networking_send((char*)(memes), meme_size, id);

	return 1;
}

int main(void)
{

	// pipe
	int fd[2];
	if (pipe(fd) == -1)
	{
		fprintf(stderr, "Error: pipe\n");
		return -1;
	}

	int pid = fork();
	if (pid < 0)
	{
		fprintf(stderr, "Error: fork\n");
		return -1;
	}

	// Child process (terminal input)
	if (pid == 0)
	{	

		u = create_empty_universe();
		int error = parse_initialization(&u, "./example_db/language_example_clean.txt");
		if (error)
		{
			fprintf(stderr, "%s\n", "Error, parse_initialization");
			return -1;
		}

	 	size_t universe_size = u.key_values.length;
	 	db = createEmptyDB(universe_size, 1000, 100);
	 	db_addParsedData(db, &u);
	 	free_universe(&u);

	 	query q = create_empty_query();
	 	error = parse_query(&q, "SELECT name, population FROM inSweden;");
	 	if (error)
	 	{
	 		fprintf(stderr, "%s\n", "Error, parse_query");
	 		return -1;
	 	}

	 	//QueryReturn* db_run_query(Database* db, query* q);
	 	QueryReturn* ret = db_run_query(db, &q);
	 	printf("length: %ld\n", ret->dataLength);

	 	//char* cities[256] = (char*)ret->columns[0].data;
	 	char* long_string 	= (char*)ret->columns[0].data;
	 	uint64_t* pops 		= (uint64_t*)ret->columns[1].data;

	 	for (uint64_t i = 0; i < ret->dataLength; i++)
	 	{
	 		printf("%s, %ld\n", &long_string[i*256], pops[i]);
	 	}

	 	free_query(&q);

		printf("DONE INITIALIZING\n");
		dbms_networking_initialize(8080, &test_function);
		dbms_networking_add_pipe_client(fd[PIPE_READ_END], fd[PIPE_WRITE_END]);

		dbms_start();
		dbms_networking_kill();
	}
	// Parent process (start server)
	else
	{
		printf("Parent! pid: %d\n", pid);

		char str[128];
		fgets(str, 128, stdin);
		str[strlen(str)-1] = '\0';

		while(str[0] != 'q')
		{
			write(fd[1], str, strlen(str)+1);

			fgets(str, 128, stdin);
			str[strlen(str)-1] = '\0';
		}

		int status, cid;
        cid = wait(&status);
        if (cid != pid)
        {
        	perror("wait");
        	return -1;
        }
        printf("Cool!\n");
	}

	return 0;
}