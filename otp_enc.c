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

#include "clientfunctions.h"

/*------------------    MAIN   --------------------*/

int main(int argc, char* argv[]) {

	if (argc < 4) { fprintf(stderr, "USAGE: %s [plaintext] [key] [port]\n", argv[0]); exit(0); }

	getTranslationFromServer(argv[1], argv[2], atoi(argv[3]), "encode");

	return 0;
}
