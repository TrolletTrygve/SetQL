#include <stdio.h>
#include <unistd.h>

#ifdef _WIN32
  	#ifndef _WIN32_WINNT
    	#define _WIN32_WINNT 0x0501  /* Windows XP. */
  	#endif
  	#include <winsock2.h>
  	#include <Ws2tcpip.h>
#else
  	#include <sys/socket.h>
 	#include <arpa/inet.h>
  	#include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
#endif

#include "dbms_networking.h"

#define DEFAULT_PORT 8080

static int server_fd;
static int client_sock;

static struct sockaddr_in address;


/**
 * 	sock_init()
 * 
 * 	Initializes sockets for windows and does nothing in POSIX platforms
 * 
 * 	Returns status from WSAStartup;
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

static int sock_quit(void)
{
	#ifdef _WIN32
	 	return WSACleanup();
	#else
	  	return 0;
	#endif
}


/**
 * 	dbms_networking_initialize(port)
 * 
 * 	Initialize the server sockets with the desired port
 * 
 * 	Argument 0:	The desired port to use when connecting to the server
 * 	Returns:	1 on success, 0 on fail
 */
int dbms_networking_initialize(uint16_t  port)
{
	int res, opt, opt_name, size;
	char buffer[1024];

	sock_init();

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		perror("socket");
		return 0;
	}

	opt = 1;
	opt_name = SO_REUSEADDR;// | SO_REUSEPORT;
	size = sizeof(opt);
	res = setsockopt(server_fd, SOL_SOCKET,opt_name, (void*)&opt, size);
	if (res)
	{
		perror("setsockopt");
		return 0;
	}

	address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    size = sizeof(struct sockaddr_in);
    res = bind(server_fd, (struct sockaddr*)&address, size);
    if (res < 0)
    {
    	perror("bind");
    	return 0;
    }

    res = listen(server_fd, 3);
    if (res < 0)
    {
    	perror("listen");
    	return 0;
    }

    size = sizeof(struct sockaddr_in);
    socklen_t* addrlen = (socklen_t*)(&size);
    client_sock = accept(server_fd, (struct sockaddr*)&address, addrlen);
    if (client_sock < 0)
    {
    	perror("accept");
    	return 0;
    }

    int valread = recv(client_sock, buffer, 1024, 0);
    if (valread < 0)
    {
    	perror("recv");
    	return 0;
    } 
    buffer[valread] = '\0';

    printf("%s\n", buffer);

    sock_quit();

    return 1;
}