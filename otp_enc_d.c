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


#define true 1
#define false 0
#define CHILD_TOTAL 5
#define BUF_LEN 256


//GLOBALS
pid_t child_pids[CHILD_TOTAL];


// PROTOTYPES
char* encryptMessage(char* messageIn) ;
void communicateWithClient(int establishedConnectionFD) ;
void catchSIGCHLD(int signo, siginfo_t* info, void* vp) ;
void addChildProcess( pid_t pid);
int removeChildProcess( pid_t pid);
int isChildProcess( pid_t pid);

// Error function used for reporting issues
void error(const char *msg) { fprintf(stderr,msg); exit(1); } 


/*-------------    MAIN    --------------*/


int main(int argc, char* argv[]) {

	//process variables
	pid_t spawnId;

	//SIGCHLD signal catching to kill zombie child
	struct sigaction SIGCHLD_action = {0};

	sigfillset(&SIGCHLD_action.sa_mask);
	SIGCHLD_action.sa_flags = SA_SIGINFO | SA_RESTART;
	SIGCHLD_action.sa_sigaction = catchSIGCHLD;
	sigaction(SIGCHLD, &SIGCHLD_action, NULL);

	//network variables
	int listenSocketFD, establishedConnectionFD, portNumber;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	// Check usage & arg
	if (argc < 2) { fprintf(stderr,"USAGE: %s [port]\n", argv[0]); exit(1); } 
	portNumber = atoi(argv[1]); 
	
	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress));
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

	while(1) {

		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr*)&clientAddress, &sizeOfClientInfo);
		if (establishedConnectionFD < 0 ) { error("ERROR on accept");}	

		//fork child processes
		spawnId = fork();
		switch(spawnId) {
			case -1:
				error("ERROR: process fork error");
				break;
			case 0:
				printf("SERVER: Received new connection\n"); fflush(stdout);
				communicateWithClient(establishedConnectionFD) ;
				return 0;
			default:
				addChildProcess(spawnId);
				break;
		}
	}
	return 0;
}



/*-------------------      FUNCTIONS     --------------------*/

/*
	kills dead child processes
*/
void catchSIGCHLD(int signo, siginfo_t* info, void* vp) {
	pid_t child_pid;
	int childExitMethod = -5;
	int result;

	//check if calling pid is a child process
	if ( isChildProcess(info->si_pid) ) {
		//get status of calling process
		child_pid = info->si_pid;
		do { 
			errno = 0;
			result = waitpid(child_pid, &childExitMethod, WNOHANG);
		} while (errno == EINTR && result == -1);

		if (result == child_pid) {
			//if child exited then need call sigterm to kill zombie
			if( WIFEXITED(childExitMethod) != 0 ) {
				removeChildProcess(child_pid);
				kill(child_pid, SIGTERM);
			}
		}
	}
}

//BG process array functions
void addChildProcess( pid_t pid) {
    int i;
    for (i=0; i < CHILD_TOTAL; i++) {
        if ( child_pids[i] == 0) {
            child_pids[i] = pid;
            break;
}   }   }
int removeChildProcess( pid_t pid) {
    int i;
    for (i=0; i < CHILD_TOTAL; i++) {
        if ( child_pids[i] == pid) {
            child_pids[i] = 0;
            return true;
    }   }
    return false;
}
int isChildProcess( pid_t pid) {
    int i;
    for (i=0; i < CHILD_TOTAL; i++) {
        if ( child_pids[i] == pid) {
            return true;
    }   }
    return false;
}


/*
	receives message with key separated by @, 
	sends back encrypted message and closes connection
*/
void communicateWithClient(int establishedConnectionFD) {
	
	int charsRead, packetLength, messageSize, charsSent;
	char buffer[BUF_LEN + 1];
	char* messageIn;
	char* messageOut;
	
	//initialize incomming message to empty string
	messageSize = 2*sizeof(buffer) + 1;
	messageIn = malloc( messageSize);
	memset(messageIn, '\0', messageSize);
	
	charsRead = -1; 
			
	//receive message
	while( strstr(messageIn, "##") == NULL ){

printf("SERVER: calling recv from socket...\n");

		memset(buffer, '\0', sizeof(buffer) );
		charsRead = recv( establishedConnectionFD, buffer, BUF_LEN, 0);
printf("charsRead: %d\n", charsRead);
		if (charsRead < 0) { error("ERROR reading from socket");}

		//make sure input is read
		do {
			ioctl( establishedConnectionFD, FIONREAD, &packetLength );
printf("%d ", packetLength);
		} while (packetLength > 0);

		//cat buffer to messageIn
		if( messageSize <= BUF_LEN + strlen(messageIn) ) {
			messageSize *= 2;
			messageIn = realloc(messageIn, messageSize);
		}
		strcat(messageIn, buffer);
printf("SERVER: message: %s\n",messageIn);
	} 

	char* terminalLocation = strstr(messageIn, "##"); // Where is the terminal
	terminalLocation[0] = '\0'; // End the string early to wipe out the terminal

	messageOut = encryptMessage(messageIn);

printf("SERVER: sending message: %s\n");
	//send encrypted message
	charsSent = send(establishedConnectionFD, messageOut, strlen(messageOut), 0);
	if (charsSent < 0 ) { error("ERROR writing to socket"); }

	//make sure output is sent
	packetLength = -5;
	do {
		ioctl( establishedConnectionFD, TIOCOUTQ, &packetLength );
	} while (packetLength > 0);

	free(messageIn);
	free(messageOut);

	close(establishedConnectionFD);
}



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
}



//	cipher helper function
int cipher_ctoi( char c) {
	if( c == ' ') 
		return 26;
	else 
		return c - 'A';
}

//	encryptMessage helper function
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


/*
	splits and incoming message with the @ symbol and encrypts the first half with the second
	returns pointer to encrypted message
*/
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

