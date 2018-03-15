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

void communicateEncoding(int establishedConnectionFD);

char* receiveClientMessage(int establishedConnectionFD);

void sendClientMessage(int establishedConnectionFD, char* messageOut);

int createListenSocket(struct sockaddr_in* serverAddress, int portNumber);




#endif
