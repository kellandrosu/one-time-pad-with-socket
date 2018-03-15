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

#define true 1
#define false 0
#define BUF_LEN 256


//PROTOTYPES
int hasIllegalChars (char* text) ;
char* readFile(char* filename) ;
void sendPayload(int sockFD, char* payload) ;

// Error function used for reporting issues
void error(const char *msg) { perror(msg); exit(0); }


/*------------------    MAIN   --------------------*/

int main(int argc, char* argv[]) {

	int socketFD, portNumber, charsWritten, charsRead, textLength;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char* serverHost = "os1";
	char* plaintext;
	char* key;
	char* payload;
	char buffer[BUF_LEN];

	if (argc < 4) { fprintf(stderr, "USAGE: %s [plaintext] [key] [port]\n", argv[0]); exit(0); }

	portNumber = atoi(argv[3]);
	memset( (char*)&serverAddress, '\0', sizeof(serverAddress));

	//setup server address struct
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	
	serverHostInfo = gethostbyname(serverHost);
	
	if (serverHostInfo < 0 ) { error("CLIENT: ERROR, no such host\n");}

	//copy in the address
	memcpy( (char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);

	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD < 0) {error("CLIENT: ERROR opening socket");}

	if ( connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0 ) {
		error("CLIENT: ERROR connection");
	}

	//read files
	plaintext = readFile(argv[1]);
	key = readFile(argv[2]);
	textLength = strlen(plaintext);
	payload = NULL;
	//separate 
	asprintf(&payload, "%s@%s##", plaintext, key);
	free(plaintext);
	free(key);

printf("CLIENT: sending %s\n", payload);

	//send message
	sendPayload(socketFD,  payload) ;

	//get return message
	memset(payload, '\0', textLength + 1);
	charsRead = recv(socketFD, payload, textLength, 0);
	if (charsRead < 0) { error("CLIENT: ERROR reading from socket");}

	//make sure data is read from socket
	int packetLength;
	do {
        ioctl( socketFD, FIONREAD, &packetLength);
    } while (packetLength > 0);
	
	fprintf(stdout, payload);

	free(payload);
	
	close(socketFD);
	
	return 0;
}



/*-------------    FUNCTIONS    --------------*/


void sendPayload(int sockFD, char* payload) {

	//number of buffer increments to store payload
	int numChunks =  (( strlen(payload) / BUF_LEN) + 1); 
	char* sendBuffer = malloc( BUF_LEN * numChunks );

	strcpy(sendBuffer, payload);

	char* sendPtr = sendBuffer;

	int i, charsSent;
	for (i=0; i<numChunks; i++) {

		charsSent = send(sockFD, sendPtr, BUF_LEN, 0);

		if (charsSent < 0) { error("CLIENT: ERROR writing to socket");}
		else if ( charsSent < BUF_LEN ) { 
			printf("CLIENT: WARNING: Not all data written to socket!\n");
		}

		//make sure message sent
		int checkSend = -5;
		do {
			ioctl(sockFD, TIOCOUTQ, &checkSend);
		} while (checkSend > 0);

		//increment sendBuffer
		sendPtr += BUF_LEN * sizeof(char);

	}
printf("CLIENT: %d payload's sent!\n", i);
	free(sendBuffer);
}


/*
*	iterates through string. returns true IFF all characters are upper case or space
*/
int hasIllegalChars (char* text) {
	int i;
	int l = strlen(text);
	for(i=0; i < l; i++){
		if( text[i] != ' ' && text[i] < 'A' && text[i] > 'Z') {
			return true;
		}
	}
	return false;
}


/*              char* readFile(char)
* 	returns a string stored in memory of a given file
*/
char* readFile(char* filename) {

    //file reading code from http://www.fundza.com/c4serious/fileIO_reading_all/
    char* payload;
    long numBytes;
    FILE* readfile = fopen(filename, "r");

    //get filelength
    fseek(readfile, 0, SEEK_END);
    numBytes = ftell(readfile);

    payload = (char*)malloc( (numBytes+1) * sizeof(char));

    fseek(readfile, 0, SEEK_SET);   //reset pointer
    fread(payload, sizeof(char), numBytes, readfile);

    fclose(readfile);

    //null terminate string and remove '\n'
    payload[numBytes-1] = '\0';

	if ( hasIllegalChars(payload) ) {
		fprintf(stderr, "%s contains illegal charactersi\n");
		exit(1);
	}

    return payload;
}
