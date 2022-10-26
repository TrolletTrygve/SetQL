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
	 	
	 	

		pid = (int)getpid();
		printf("Child! pid: %d\n", pid);
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