#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#if defined(_WIN32)

#include <winsock2.h>

int create_socket(char* destHost, short destPort, char* res)
{
	WSADATA wsa;
	SOCKET s;

	// printw("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		// printw("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		// printw("Could not create socket : %d", WSAGetLastError());
		return 1;
	}

	// printw("Socket created.\n");
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(destPort);
	server.sin_addr.s_addr = inet_addr(destHost);

	// Connect to remote server
	int connection_status = connect(s, (struct sockaddr *)&server, sizeof(server));
	if (connection_status != 0) {
		return connection_status;
	}

	// Receieve data from the server, load into res string
	recv(s, res, sizeof(res), 0);

	// Close socket
	close(s);

	return 0;
}

#else

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <unistd.h>

// #include <ncurses.h>

int create_socket(char addr[], int port, char *res)
{
	// Create a socket
	int s = socket(AF_INET, SOCK_STREAM, 0);

	// Specify address:port dest
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(addr);

	int connection_status = connect(s, (struct sockaddr *)&server, sizeof(server));

	// Connect to remote server
	if (connection_status != 0)
	{
		// puts("connect error");
		return connection_status;
	}

	// Receieve data from the server, load into res string
	recv(s, res, sizeof(res), 0);

	// Print response
	// printw("The server responded with: %s\n", res);

	// Close socket
	close(s);

	return 0;
}

#endif