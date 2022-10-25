#ifndef __DBMS_NETWORKING_H__
#define __DBMS_NETWORKING_H__

#include <stdint.h>

#define MAX_MESSAGE_SIZE 128

typedef int client_id;

// message handler function definition
typedef int (*message_func_p)(char[MAX_MESSAGE_SIZE], int, client_id);

// Function declarations
int dbms_networking_initialize(uint16_t port, message_func_p message_handler);
int dbms_networking_add_pipe_client(int read_fd, int write_fd);

int dbms_networking_send(char* buffer, int size, client_id id);

int dbms_start(void);
int dbms_networking_kill(void);

#endif