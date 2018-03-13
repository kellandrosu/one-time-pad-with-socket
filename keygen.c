#include <stdio.h>
#include <stdlib.h>
#include <time.h>


char randomChar() {
	//random number between 0 and 26 inclusive
	int randInt = rand() % 27;
	if( randInt == 26 ) 
		return ' ';
	else
		return (char)randInt + 'A';
}

int main(int argc, char* argv[]) {

	int i;
	int keylength;
	char* key;

	if ( argc != 2) {
		fprintf(stderr, "Error - Usage: keygen [int]\n");
		exit(1);
	}

	keylength = atoi(argv[1]);

	if( keylength == 0 ) {
		fprintf(stderr, "Error: %s is not a valid key length\n", argv[1]);
		exit(1);
	}

	//seed rand
	srand(time(NULL));

	//initialize key with extra space for null term
	key = malloc( (keylength + 1) * sizeof(char) );

	for( i=0; i < keylength; i++) {
		key[i] = randomChar();
	}
	key[keylength] = '\0';

	fprintf(stdout, "%s\n", key);

	return 0;
}



