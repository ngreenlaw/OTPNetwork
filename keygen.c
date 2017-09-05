/*Nathan Greenlaw
 * CS 344
 * Program 4: keygen: Generate a random key to use as a cipher*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[])
{	
	srand(time(NULL));
	char key;
	char* len = argv[1];
	int l = atoi(len); // length of the keygen file
	int i=0;
	// take a random number from 0 to 26 to represent A-Z and the blank space and add 65 to it unless it is the blank space
	for(i; i < l; i++)
	{
		int r = rand()%27;
		if( r == 26)
		{
			r = 32;
		}
		else
		{
			r+=65;
		}
		key = (char)r;// the ascii value of the key converted to a char
		printf("%c",key);
	}
	printf("\n");
	return 0;
}
