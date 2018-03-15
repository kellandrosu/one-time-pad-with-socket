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
#define BUF_LEN 256



//PROTOTYPES
int hasIllegalChars (char* text) ;
char* readFile(char* filename) ;
void sendPayload(int sockFD, char* payload) ;
void getTranslationFromServer(char* plainTextFile, char* keyFile, int portNumber, char* method);



#endif
