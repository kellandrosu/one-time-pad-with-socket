/*
    Author:      Andrius Kelly
    Date:        March 13, 2018
    Description: CS 344 Project 4
	Note:		Network code modified from server.c file provided.
*/

#ifndef SERVER_FUNC
#define SERVER_FUNC


#define true 1
#define false 0
#define CHILD_TOTAL 5
#define BUF_LEN 256


// PROTOTYPES

char enc_cipher(char msg_c, char key_c);
char dec_cipher(char msg_c, char key_c);

void recvTranslateSend(int establishedConnectionFD, char (*cipherFunc)(char, char));

char* receiveClientMessage(int establishedConnectionFD);

void sendClientMessage(int establishedConnectionFD, char* messageOut);

int createListenSocket(struct sockaddr_in* serverAddress, int portNumber);




#endif
