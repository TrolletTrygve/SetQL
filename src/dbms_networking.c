#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifdef _WIN32
  	#ifndef _WIN32_WINNT
    	#define _WIN32_WINNT 0x0501
  	#endif
  	#include <winsock2.h>
  	#include <Ws2tcpip.h>
#else
  	#include <sys/socket.h>
	#include <sys/select.h>
	#include <sys/ioctl.h>
 	#include <arpa/inet.h>
  	#include <netdb.h>
#endif

#include "dbms_networking.h"

#define MAX_CLIENTS 	8
#define DEFAULT_PORT 	8080

#define CLIENT_TYPE_SOCKET	0
#define CLIENT_TYPE_PIPE	1

static int dbms_started = 0;
static struct sockaddr_in address;
static int server_socket;

// Client information
static struct sockaddr_in client_addresses[MAX_CLIENTS];
	
static int client_types		[MAX_CLIENTS];
static int client_read_fds	[MAX_CLIENTS];
static int client_write_fds	[MAX_CLIENTS];
static int client_count;

static fd_set readfds;

static message_func_p message_function;

/**
 * 	sock_init()
 * 
 * 	Initializes sockets for windows and does nothing in POSIX platforms
 * 
 * 	Returns: status from WSAStartup
 */
static int sock_init(void)
{
	#ifdef _WIN32
    	WSADATA wsa_data;
    return WSAStartup(MAKEWORD(1,1), &wsa_data);
  	#else
    	return 0;
  	#endif
}


/**
 * 	sock_quit()
 * 
 * 	quits sockets for windows and does nothing on POSIX platforms
 * 
 * 	Returns: status for WSACleanup
 */
static int sock_quit(void)
{
	#ifdef _WIN32
	 	return WSACleanup();
	#else
	  	return 0;
	#endif
}


/**
 * 	accept_connection()
 * 
 * 	Accepts an incomming connection on the server client
 * 	and sends it a welcome message.
 * 
 * 	returns:	1 on success, 0 on fail.
 */
static int accept_connection(void)
{
	int new_client;

	int struct_size 	= sizeof(struct sockaddr_in);
    socklen_t* addrlen 	= (socklen_t*)(&struct_size);

    struct sockaddr_in incomming_address;
    struct sockaddr* incomming_address_p = (struct sockaddr*)&incomming_address;

    new_client = accept(server_socket, incomming_address_p, addrlen);
    if (new_client < 0)
    {
    	perror("accept");
    	return 0;
    }

    char str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &incomming_address.sin_addr, str, INET_ADDRSTRLEN);
	printf("%s\n", str);

	printf("New connection from %s!\n", str);
	
	/*
    const char* msg = "Hejsan!";
    int result = send(new_client, msg, strlen(msg), 0);
    if (result != (int)strlen(msg))
    {
    	perror("send");
    	return 0;
    }
    */

    client_addresses[client_count] 	= incomming_address; 
   	
   	client_types[client_count]		= CLIENT_TYPE_SOCKET;
    client_read_fds[client_count] 	= new_client;
    client_write_fds[client_count] 	= new_client;
    FD_SET(new_client, &readfds);
    client_count++;

	return 1;
}


/**
 *	kill_client(index)
 * 
 * 	closes a client socket and reorganizes the socket array
 * 	
 * 	argument 0:	The index of the socket to kill
 * 	returns:	1 on success 0 on fail
 */
static int kill_client(client_id id)
{
	switch(client_types[id])
	{
		case CLIENT_TYPE_SOCKET:
			#ifdef _WIN32
				if (closesocket(client_read_fds[id]) != 0)
				{
					fprintf(stderr, "Error: closesocket\n");
					return 0;
				}
			#else
				if (close(client_read_fds[id]) == -1)
				{
					perror("close");
					return 0;
				}
			#endif
		break;

		case CLIENT_TYPE_PIPE:
			if (close(client_read_fds[id]) == -1)
			{
				perror("read pipe close");
				return 0;
			}
			if (close(client_write_fds[id]) == -1)
			{
				perror("rwite pipe close");
				return 0;
			}
		break;

		default:
			fprintf(stderr, "%s\n", "Error: unknown client type");
			return 0;
		break;
	}

	FD_CLR(client_read_fds[id], &readfds);

	if (id == 0)
		client_read_fds[id] = 0;
	else
	{
		client_types[id]		= client_types[client_count-1];
		client_read_fds[id] 	= client_read_fds[client_count-1];
		client_write_fds[id]	= client_write_fds[client_count-1];
		client_addresses[id]	= client_addresses[client_count-1];
	}
	client_count--;

	return 1;
}


/**
 * 	handle_message(index)
 * 
 * 	Recieves a message from a socket who is ready to be read.
 * 	if the message is empty, we can assume that the socket has been closed
 * 	on the other end and we close it.
 * 
 * 	argument 0:	The index of the socket to recieve data from
 * 	returns:	1 on success, 0 on fail
 */
static int handle_message(client_id id)
{
	char buffer[128];
	int result;

	// Checks the client type and handles the message accordingly
	switch(client_types[id])
	{
		case CLIENT_TYPE_SOCKET:
			result = recv(client_read_fds[id], buffer, 128, 0);
		break;
		case CLIENT_TYPE_PIPE:
			result = read(client_read_fds[id], buffer, 128);
		break;
		default:
			fprintf(stderr, "%s\n", "Error: unknown client type");
			return 0;
		break;
	}

	if (result < 0)
	{
		perror("recv");
		return  1;
	}
	if (result == 0)
	{
		if (!kill_client(id))
		{
			fprintf(stderr, "%s\n", "Error: kill_client");
			return 0;
		}
		return 1;
	}
	buffer[result] = '\0';

	// Call message handle function
	(*message_function)(buffer, result, id);

	return 1;
}


/**
 * 	dbms_networking_initialize(port)
 * 
 * 	Initialize the server sockets with the desired port and makes it listen
 * 
 * 	Argument 0:	The desired port to use when connecting to the server
 * 	Returns:	1 on success, 0 on fail
 */
int dbms_networking_initialize(uint16_t port, message_func_p message_handler)
{
	int res, opt, opt_name, size;
	sock_init();

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		perror("socket");
		return 0;
	}

	FD_ZERO(&readfds);
	FD_SET(server_socket, &readfds);

	opt = 1;
	opt_name = SO_REUSEADDR;
	size = sizeof(opt);
	res = setsockopt(server_socket, SOL_SOCKET,opt_name, (void*)&opt, size);
	if (res)
	{
		perror("setsockopt");
		return 0;
	}

	#ifdef _WIN32
		res = ioctlsocket(server_socket, FIONBIO, (void*)&opt);
		if (res != 0)
		{
			fprintf(stderr, "Error: ioctlsocket with code: %d\n", res);
			return 0;
		}
	#else
		res = ioctl(server_socket, FIONBIO, (void*)&opt);
		if (res < 0)
		{
			perror("ioctl");
			return 0;
		}
	#endif
	
	
	address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    size = sizeof(struct sockaddr_in);
    res = bind(server_socket, (struct sockaddr*)&address, size);
    if (res < 0)
    {
    	perror("bind");
    	return 0;
    }

    res = listen(server_socket, 4);
    if (res < 0)
    {
    	perror("listen");
    	return 0;
    }

    message_function = message_handler;
    dbms_started = 1;
    return 1;
}

/**
 * 	dbms_networking_add_pipe_client(read_fd, write_fd)
 * 	
 * 	Adds a pipe client to be checked for input just as a socket
 * 	This allows for input to the server from the terminal
 * 	
 * 	argument 0:	The pipe file descriptor to read from
 * 	argument 1: The pipe file descriptor to write to
 * 	returns:	1 on success, 0 on fail
 */ 
int dbms_networking_add_pipe_client(int read_fd, int write_fd)
{
	if (!dbms_started)
	{
		fprintf(stderr, "%s", "Error: dbms_networking_add_pipe_client, ");
		fprintf(stderr, "%s\n", "DBMS has not been initialized");
		return 0;
	}

	if (client_count+1 >= MAX_CLIENTS)
	{
		fprintf(stderr, "%s", "Error: dbms_networking_add_pipe_client, ");
		fprintf(stderr, "%s\n", "Too many clients");
		return 0;
	}

	client_types  		[client_count] = CLIENT_TYPE_PIPE;
	client_read_fds		[client_count] = read_fd;
	client_write_fds	[client_count] = write_fd;
	client_count++;

	return 1;
}


/**
 * 	dbms_start()
 * 	
 * 	Start listening to the sockets, accepting connections
 * 	and handeling messages.
 * 
 * 	returns:	1 on successful exit, 0 on fail.
 */
int dbms_start(void)
{
	if (!dbms_started)
	{
		fprintf(stderr, "%s\n", "Error: dbms_start");
		fprintf(stderr, "%s\n", "DBMS has not been initialized");
		return 0;
	}

	int ready = 0, should_exit = 0, i;
	while (!should_exit)
	{
		// Clear and insert to FD_SET and find the highest socket number
		int max_sd = server_socket;

		FD_ZERO(&readfds);
		FD_SET(server_socket, &readfds);
		for (i = 0; i < client_count; i++)
		{
			if (client_read_fds[i] > 0)
				FD_SET(client_read_fds[i], &readfds);

			if (client_read_fds[i] > max_sd)
				max_sd = client_read_fds[i];
		}

		ready = select(max_sd+1, &readfds, NULL, NULL, NULL);

		if (ready == -1)
		{
			perror("select");
			return 0;
		}

		if (FD_ISSET(server_socket, &readfds))
		{
			if (!accept_connection())
			{
				fprintf(stderr, "%s\n", "Error: accept_connection");
				return 0;
			}
		}

		for (i = 0; i < client_count; i++)
		{
			if (FD_ISSET(client_read_fds[i], &readfds))
			{
				printf("something happened\n");
				if (!handle_message(i))
				{
					fprintf(stderr, "%s\n", "Error: handle_message");
					return 0;
				}
			}
		}
	}

	return 1;
}


/**
 * 	dbms_networking_kill()
 * 
 * 	Frees memory and closes sockets
 * 	(REQUIRES THAT INITIALIZE HAS BEEN CALLED)
 * 
 * 	returns: kill status
 */
int dbms_networking_kill(void)
{
	if (!dbms_started)
	{
		fprintf(stderr, "%s\n", "Error: dbms_networking_kill");
		fprintf(stderr, "%s\n", "DBMS has not been initialized");
		return 0;
	}

	#ifdef _WIN32
		closesocket(server_socket);	
	#else
		close(server_socket);	
	#endif

	dbms_started = 0;
	return sock_quit();
}