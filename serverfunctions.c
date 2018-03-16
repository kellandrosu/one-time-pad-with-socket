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
#include <signal.h>
#include <errno.h>

#include "serverfunctions.h"



/*-------------------      FUNCTIONS     --------------------*/



/*
		creates and returns a socket for listening
*/
int createListenSocket(struct sockaddr_in* serverAddress, int portNumber) {
	int listenSocketFD;

	//zero out serv addr struct
	memset((char *)serverAddress, '\0', sizeof(*serverAddress));

	// Set up the address struct for this process (the server)
	serverAddress->sin_family = AF_INET;
	serverAddress->sin_port = htons(portNumber);
	serverAddress->sin_addr.s_addr = INADDR_ANY;

	//set up listensocket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocketFD < 0 ) { error("ERROR opening socket"); }

	//bind socket to port
	if ( bind (listenSocketFD, (struct sockaddr*)serverAddress, sizeof(*serverAddress)) < 0 ) {
		error("ERROR on binding");
	}
	return listenSocketFD;
}



//	cipher helper function
int cipher_ctoi( char c) {
	if( c == ' ') 
		return 26;
	else 
		return c - 'A';
}


//	encryptMessage helper function
char dec_cipher(char msg_c, char key_c) {
	int m, k, cipherInt;

	//get int values for msg_c and key_c
	m = cipher_ctoi(msg_c);
	k = cipher_ctoi(key_c);

	cipherInt = (m - k) % 27;

	if (cipherInt < 0 ) {
		cipherInt += 27;
	}

	if( cipherInt == 26 )
		return ' ';
	else
		return cipherInt + 'A';
}

//	encryptMessage helper function
char enc_cipher(char msg_c, char key_c) {
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


/*
	splits and incoming message with the @ symbol and applies the cipher
	returns pointer to encrypted message
*/
char* encryptMessage( char* messageIn, char (*cipherFunc)(char, char)) {

	int i;
	char* messageOut;
	//set keyToken to point to start of key
	char* keyToken = strtok(messageIn, "@");
	keyToken = strtok(NULL, "@");
	messageOut = (char*)malloc( strlen(messageIn + 1));

	for( i=0; i<strlen(messageIn); i++ ) {
		messageOut[i] = cipherFunc(messageIn[i], keyToken[i]); 
	}

	messageOut[strlen(messageIn)] = '\0';

	return messageOut;
}	


/*
* 	receives, encodes and sends client message
*/
void recvTranslateSend( int establishedConnectionFD, char (*cipherFunc)(char, char) ){

	char* messageIn ;
	char* messageOut;
	
	messageIn = receiveClientMessage(establishedConnectionFD);

	//printf("SERVER: message received size: %i\n", strlen(messageIn)); fflush(stdout);
	
	messageOut = encryptMessage(messageIn, cipherFunc);

	//printf("SERVER: sending message size: %i\n", strlen(messageOut)); fflush(stdout);
	
	sendClientMessage(establishedConnectionFD,  messageOut) ;

	//printf("SERVER: messaget sent!\n"); fflush(stdout);

	free(messageIn);
	free(messageOut);

	return;
}




/*
	receives message and null terminates terminal signal ## 
*/
char* receiveClientMessage(int establishedConnectionFD) {
	
	int charsRead, packetLength, messageSize;
	char buffer[BUF_LEN + 1];
	char* messageIn;
	
	//initialize incomming message to empty string
	messageSize = 2*sizeof(buffer) + 1;
	messageIn = malloc(messageSize);
	memset(messageIn, '\0', messageSize);
			
	//receive message
	while( strstr(messageIn, "##") == NULL ){

//printf("SERVER: reading...\n");

		memset(buffer, '\0', sizeof(buffer) );
		charsRead = 0;

		while (charsRead >= 0 && charsRead < BUF_LEN ){
			charsRead += recv( establishedConnectionFD, buffer, BUF_LEN, 0);
		}
		
		if (charsRead < 0) { 
			fprintf(stderr, "ERROR could not read\n");
		}

//		else if (charsRead < BUF_LEN ) {
//			fprintf(stderr, "ERROR read length smaller than buffer\n");
//			fflush(stderr);
//			//continue reading rest of buffer
//			continue;
//		}

		//make sure input is read
//		do {
//			ioctl( establishedConnectionFD, FIONREAD, &packetLength );
//		} while (packetLength > 0);

		//cat buffer to messageIn
		if( messageSize <= BUF_LEN + strlen(messageIn) ) {
			messageSize *= 2;
			messageIn = realloc(messageIn, messageSize);
		}
		strcat(messageIn, buffer);
	} 

	char* terminalLocation = strstr(messageIn, "##"); // Where is the terminal
	terminalLocation[0] = '\0'; //replace terminal location 

//printf("SERVER: message read size: %i\n",strlen(messageIn)); fflush(stdout);

	return messageIn;
}


/*
*	sends client message
*/
void sendClientMessage(int establishedConnectionFD, char* messageOut) {

	int charsSent;

	//send encrypted message
	charsSent = send(establishedConnectionFD, messageOut, strlen(messageOut), 0);
	if (charsSent < 0 ) { fprintf(stderr,"ERROR writing to socket\n"); }

	//make sure output is sent
	int packetLength = -5;
	do {
		ioctl( establishedConnectionFD, TIOCOUTQ, &packetLength );
	} while (packetLength > 0);
//printf("SERVER: sent size: %i\n", charsSent);
}
