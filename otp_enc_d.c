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


//GLOBALS
pid_t child_pids[CHILD_TOTAL];



/* -------  PROCESS MANAGEMENT FUNCTIONS ------ */

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

/*
*   process id array functions
*/
void addChildProcess( pid_t pid) {
    int i;
    for (i=0; i < CHILD_TOTAL; i++) {
        if ( child_pids[i] == 0) {
            child_pids[i] = pid;
            break;
}}}
int removeChildProcess( pid_t pid) {
    int i;
    for (i=0; i < CHILD_TOTAL; i++) {
        if ( child_pids[i] == pid) {
            child_pids[i] = 0;
            return true;
    }} return false;
}
int isChildProcess( pid_t pid) {
    int i;
    for (i=0; i < CHILD_TOTAL; i++) {
        if ( child_pids[i] == pid) {
            return true;
    }} return false; 
}



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
	int listenSocketFD, connectionFD, portNumber;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	// Check usage & arg
	if (argc < 2) { fprintf(stderr,"USAGE: %s [port]\n", argv[0]); exit(1); } 
	portNumber = atoi(argv[1]); 
	
	listenSocketFD = createListenSocket( &serverAddress, portNumber);

	//open socket to listen with queue length of 5
	listen( listenSocketFD, 5); 

	char* clientType;

	sizeOfClientInfo = sizeof(clientAddress);

	while(1) {

		connectionFD = accept(listenSocketFD, (struct sockaddr*)&clientAddress, &sizeOfClientInfo);
		
		if (connectionFD < 0 ) { 
			fprintf(stderr, "ERROR on accept\n");
		}
		else {
			//fork child processes
			spawnId = fork();
			switch(spawnId) {
				case -1:
					fprintf(stderr, "ERROR: process fork error\n");
					exit(1);
					break;
				case 0:
					clientType = receiveClientMessage(connectionFD);
					if ( strcmp(clientType, "encode") == 0 ) { 
						sendClientMessage(connectionFD, "OK");
						recvTranslateSend(connectionFD, enc_cipher) ;
					}
					else {
						sendClientMessage(connectionFD, "NO");
					}
					close(connectionFD);
					return 0;
				default:
					addChildProcess(spawnId);
					break;
			}
		}
	}
	return 0;
}

