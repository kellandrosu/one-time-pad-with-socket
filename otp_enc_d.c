/*
    Author:      Andrius Kelly
    Date:        March 13, 2018
    Description: CS 344 Project 4
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>


#define BUF_LEN 256


void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char* argv[]) {


	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[BUF_LEN];
	struct sockaddr_in serverAddress, clientAddress;

	// Check usage & arg
	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } 

	structportNumber = atoi(argv[1]); 
	
	//zero out serv addr struct
	memset((char *)&serverAddress, '\0', sizeof(serverAddress));

	// Set up the address struct for this process (the server)
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	//set up listensocket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocketFD < 0 ) { error("ERROR opening socket"); }

	//bind socket to port
	if ( bind (listenSocketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddres)) < 0 ) {
		error("ERROR on binding");
	}

	listen( listenSocketFD, 5); //open socket to listen with queue length of 5

	sizeOfClientInfo = sizeof(clientAddress);

	while(1) {

		establishedConnectionFD = accept(listenSockFD, (struct sockaddr*)&clientAddress, &sizeOfClientInfo);
		if (establishedConnectionFD < 0 ) { error("ERROR on accept");}	

		//receive message
		memset(buffer, '\0', BUF_LEN);
		charsRead = recv( establishedConnectionFD, buffer, 255, 0);
	 	if (charsRead < 0) { error("ERROR reading from socket");}

		printf(buffer);
		fflush(stdout);

		close(establishedConnectionFD);
		
	}


	return 0;
}
