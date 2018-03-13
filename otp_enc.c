/*
    Author:      Andrius Kelly
    Date:        March 13, 2018
    Description: CS 344 Project 4
    Note:        Network code modified from client.c file provided.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>


#define BUF_LEN 256

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char* argv[]) {

	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[BUF_LEN];

	if (argc < 3) { fprintf(stderr, "USAGE: %s [hostname] [port]\n", argv[0]); exit(0); }

	portNumber = atoi(argv[2]);
	memset( (char*)&serverAddress, '\0', sizeof(serverAddress));

	//setup server address struct
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	
	serverHostInfo = gethostbyname(argv[1]);
	if (serverHostInfo < 0 ) { error("CLIENT: ERROR, no such host\n");}

	//copy in the address
	memcpy( (char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);

	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD < 0) {error("CLIENT: ERROR opening socket");}

	if ( connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0 ) {
		error("CLIENT: ERROR connection");
	}


	//get input
	printf("CLIENT: Enter text: ");
	memset( buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer) - 1, stdin);
	buffer[strcspn(buffer, "\n")] = '\0'; //replace trailing \n with \0

	//send message
	charsWritten = send(socketFD, buffer, strlen(buffer), 0);
	if (charsWritten < 0) { error("CLIENT: ERROR writing to socket");}
	else if ( charsWritten < strlen(buffer) ) { printf("CLIENT: WARNING: Not all data written to socket!\n");}
	
	int checkSend = -5;
	do {
		ioctl(socketFD, TIOCOUTQ, &checkSend);
	} while (checkSend > 0);

	//get return message
	memset(buffer, '\0', sizeof(buffer));
	charsRead = recv(socketFD, buffer, sizeof(buffer)-1, 0);
	if (charsRead < 0) { error("CLIENT: ERROR reading from socket");}
	printf("CLIENT: Server message: %s\n", buffer);

	close(socketFD);
	
	return 0;
}
