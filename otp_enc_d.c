/*
    Author:      Andrius Kelly
    Date:        March 13, 2018
    Description: CS 344 Project 4
	Note:		Network code modified from server.c file provided.
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

// PROTOTYPES
char* encryptMessage(char* messageIn) ;

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char* argv[]) {


	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, packetLength, messageSize, charsSent;
	socklen_t sizeOfClientInfo;
	char buffer[BUF_LEN];
	char* messageIn;
	char* messageOut;
	struct sockaddr_in serverAddress, clientAddress;

	// Check usage & arg
	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } 

	portNumber = atoi(argv[1]); 
	
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
	if ( bind (listenSocketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0 ) {
		error("ERROR on binding");
	}

	listen( listenSocketFD, 5); //open socket to listen with queue length of 5

	sizeOfClientInfo = sizeof(clientAddress);

	//initialize incomming message to empty string
	messageSize = 2*sizeof(buffer) + 1;
	messageIn = malloc( messageSize);
	messageIn[0] = '\0';

	while(1) {

		memset(messageIn, '\0', messageSize);

		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr*)&clientAddress, &sizeOfClientInfo);
		if (establishedConnectionFD < 0 ) { error("ERROR on accept");}	

		charsRead = -1; 
		//check incoming packet size	
		//do {
			//receive message
			memset(buffer, '\0', BUF_LEN);
			charsRead = recv( establishedConnectionFD, buffer, 255, 0);
			if (charsRead < 0) { error("ERROR reading from socket");}

			//make sure input is read
			do {
				ioctl( establishedConnectionFD, FIONREAD, &packetLength );
			} while (packetLength > 0);

			//cat buffer to messageIn
			if( messageSize <= strlen(buffer) + strlen(messageIn) ) {
				messageSize *= 2;
				messageIn = realloc(messageIn, messageSize);
			}
			strcat(messageIn, buffer);

			messageOut = encryptMessage(messageIn);

			//send encrypted message
			charsSent = send(establishedConnectionFD, messageOut, strlen(messageOut), 0);
			if (charsSent < 0 ) { error("ERROR writing to socket"); }

			//make sure input is read
			packetLength = -5;
			do {
				ioctl( establishedConnectionFD, TIOCOUTQ, &packetLength );
			} while (packetLength > 0);


		//} while( charsRead > 0 ); //read until client closes connection

		close(establishedConnectionFD);
		
	}

	free(messageOut);
	free(messageIn);

	return 0;
}



/*-------------------      FUNCTIONS     --------------------*/


int cipher_ctoi( char c) {
	if( c == ' ') 
		return 26;
	else 
		return c - 'A';
}

char cipher(char msg_c, char key_c) {
	int m, k, cipherInt;

	//get int values for msg_c and key_c
	m = cipher_ctoi(msg_c);
	k = cipher_ctoi(key_c);

	cipherInt = (m + k) % 27;

	if( cipherInt == 26 )
		return ' ';
	else
		return cipherInt + 'A';
}

char* encryptMessage(char* messageIn) {

	int i;
	char* messageOut;
printf("%s\n", messageIn);
	//set keyToken to point to start of key
	char* keyToken = strtok(messageIn, "@");
	keyToken = strtok(NULL, "@");
	messageOut = (char*)malloc( strlen(messageIn + 1));

	for( i=0; i<strlen(messageIn); i++ ) {
		messageOut[i] = cipher(messageIn[i], keyToken[i]); 
	}

	messageOut[strlen(messageIn)] = '\0';

printf("%s\n%s\n%s\n", messageIn, keyToken, messageOut);

	return messageOut;
}	

