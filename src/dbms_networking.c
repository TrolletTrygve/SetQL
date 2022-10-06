#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
//#include <sys/ioctl.h>

#ifdef _WIN32
  	#ifndef _WIN32_WINNT
    	#define _WIN32_WINNT 0x0501
  	#endif
  	#include <winsock2.h>
  	#include <Ws2tcpip.h>
#else
  	#include <sys/socket.h>
 	#include <arpa/inet.h>
	#include <poll.h>
  	#include <netdb.h>
#endif

#include "dbms_networking.h"

#define HOST_SOCKET		0
#define MAX_CLIENTS 	8
#define DEFAULT_PORT 	8080


static int dbms_started = 0;
static struct sockaddr_in address;

static int pfd_count = 0;
static struct pollfd pfds[MAX_CLIENTS];


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
	int new_client, result;

	int struct_size 	= sizeof(struct sockaddr_in);
    socklen_t* addrlen 	= (socklen_t*)(&struct_size);

    new_client = accept(pfds[0].fd, (struct sockaddr*)&address, addrlen);
    if (new_client < 0)
    {
    	perror("accept");
    	return 0;
    }

    printf("New connection!\n");

    const char* msg = "Hejsan!";
    result = send(new_client, msg, strlen(msg), 0);
    if (result != (int)strlen(msg))
    {
    	perror("send");
    	return 0;
    }

    pfds[pfd_count].fd 		= new_client;
    pfds[pfd_count].events 	= POLLIN;
    pfd_count++;

	return 1;
}


/**
 *	kill_pfd(index)
 * 
 * 	closes a client socket and reorganizes the socket array
 * 	
 * 	argument 0:	The index of the socket to kill
 * 	returns:	1 on success 0 on fail
 */
static int kill_pfd(int index)
{
	if (close(pfds[index].fd) == -1)
	{
		perror("close");
		return 0;
	}

	if (index == 0)
		pfds[index].fd = 0;
	else
		pfds[index].fd = pfds[pfd_count-1].fd;
	pfd_count--;

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
static int handle_message(int index)
{
	unsigned char buffer[128];
	int result;

	result = recv(pfds[index].fd, buffer, 128, 0);
	if (result < 0)
	{
		perror("recv");
		return  0;
	}
	if (result == 0)
	{
		if (!kill_pfd(index))
		{
			fprintf(stderr, "%s\n", "Error: kill_pfd");
			return 0;
		}
		return 1;
	}

	buffer[result] = '\0';
	printf("Message recieved: %s, with length %d\n", buffer, result);

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
int dbms_networking_initialize(uint16_t  port)
{
	int res, opt, opt_name, size;
	sock_init();

	pfds[0].fd = socket(AF_INET, SOCK_STREAM, 0);
	if (pfds[0].fd < 0)
	{
		perror("socket");
		return 0;
	}
	pfds[0].events = POLLIN;
	pfd_count = 1;

	opt = 1;
	opt_name = SO_REUSEADDR;// | SO_REUSEPORT;
	size = sizeof(opt);
	res = setsockopt(pfds[0].fd, SOL_SOCKET,opt_name, (void*)&opt, size);
	if (res)
	{
		perror("setsockopt");
		return 0;
	}


	// TODO: Make server socket non-blocking with fcntl()
	/*
	res = ioctl(pfds[0].fd, FIONBIO, (void*)&opt);
	if (res < 0)
	{
		perror("ioctl");
		return 0;
	}
	*/


	address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    size = sizeof(struct sockaddr_in);
    res = bind(pfds[0].fd, (struct sockaddr*)&address, size);
    if (res < 0)
    {
    	perror("bind");
    	return 0;
    }

    res = listen(pfds[0].fd, 4);
    if (res < 0)
    {
    	perror("listen");
    	return 0;
    }

    dbms_started = 1;
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
		printf("start\n");
		ready = poll(pfds, pfd_count, -1);
		if (ready == -1)
		{
			perror("poll");
			return 0;
		}

		printf("Ready: %d\n", ready);

		if (pfds[0].revents != 0)
		{
			if (!accept_connection())
			{
				fprintf(stderr, "%s\n", "Error, accept_connection");
				return 0;
			}
		}

		for (i = 1; i < pfd_count; i++)
		{
			if (pfds[i].revents != 0)
			{
				if (pfds[i].revents & POLLIN)
				{
					printf("something happened\n");
					if (!handle_message(i))
					{
						fprintf(stderr, "%s\n", "Error, handle_message");
						return 0;
					}
				}
				else
				{
					// something went wrong
					if (!kill_pfd(index))
					{
						fprintf(stderr, "%s\n", "Error: kill_pfd");
						return 0;
					}
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

	close(pfds[0].fd);	

	dbms_started = 0;
	return sock_quit();
}