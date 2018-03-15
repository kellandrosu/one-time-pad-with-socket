/*
    Author:      Andrius Kelly
    Date:        March 13, 2018
    Description: CS 344 Project 4
	Note:		Network code for encode/decode clients.
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

#include "clientfunctions.h"



/*-------------------      FUNCTIONS     --------------------*/

// Error function used for reporting issues
void error(const char *msg) { perror(msg); exit(0); }

/*
*	handles communication with encode and decode servers
*/
void getTranslationFromServer(char* plainTextFile, char* keyFile, int portNumber, char* method) {

	int socketFD, charsWritten, charsRead, textLength;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    char* serverHost = "localhost";
    char* plaintext;
    char* key;
    char* payload;
    char serverOK[3];
    char buffer[BUF_LEN];


    //read files
    plaintext = readFile(plainTextFile);
    key = readFile(keyFile);
    textLength = strlen(plaintext);

    if( textLength > strlen(key) ) {
        fprintf(stderr, "CLIENT: ERROR key too shorti\n");
        exit(1);
    }

    //combine plaintext and key into payload
    payload = NULL;
    asprintf(&payload, "%s@%s##", plaintext, key);
    free(plaintext);
    free(key);


    //setup server address struct
    memset( (char*)&serverAddress, '\0', sizeof(serverAddress));
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

    char* methodReq = NULL;
	asprintf(&methodReq, "%s##", method);		//insert terminal character

    //send encode verification
	sendPayload(socketFD,  methodReq) ;
    serverOK[0] = '\0';
    charsRead = recv(socketFD, serverOK, 3, 0);
    if( strcmp(serverOK, "OK") != 0 ) {
        fprintf(stderr, "Rejected by server\n");
        exit(1);
    }


//printf("CLIENT: sending %s\n", payload);

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

    fprintf(stdout, "%s\n", payload);

    free(payload);

    close(socketFD);
}


/*
*	sends a payload over a socket connection in BUF_LEN size chunks
*/
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
    free(sendBuffer);
	//printf("CLIENT: %d payload's sent!\n", i);
}


/*
*   iterates through string. returns true IFF all characters are upper case or space
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
*   returns a string stored in memory of a given file
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
