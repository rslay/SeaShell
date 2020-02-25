
#if defined(_WIN32)

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") //Winsock Library

int create_socket()
{
	// WSADATA wsa;
	// SOCKET s;

	// // printf("\nInitialising Winsock...");
	// if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	// {
	// 	// printf("Failed. Error Code : %d", WSAGetLastError());
	// 	return 1;
	// }

	// // printf("Initialised.\n");

	// if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	// {
	// 	// printf("Could not create socket : %d", WSAGetLastError());
	// 	return 1;
	// }

	// // printf("Socket created.\n");
	// struct sockaddr_in server;
	// server.sin_family = AF_INET;
	// server.sin_port = htons(4444);
	// server.sin_addr.s_addr = inet_addr("127.0.0.1");

	// //Connect to remote server
	// if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	// {
	// 	// puts("connect error");
	// 	return 1;
	// }
	// else
	// {
	// 	return 0;
	// }
}

#else

#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <fcntl.h>
#include <unistd.h>

#include <ncurses.h>

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

	// Receieve data from the server
	// char res[256]; // Instead, pass in char* res pointer
	recv(s, res, sizeof(res), 0);

	// Print response
	// printw("The server responded with: %s\n", res);

	// Close socket
	close(s);

	return 0;
}

#endif