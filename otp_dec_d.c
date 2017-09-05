/*Nathan Greenlaw
 * CS 344
 * Program 4: otp_dec_d decrypt text with a given key by receiving it from otp_dec*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/ioctl.h>


void error(const char *msg) { fprintf(stderr,"%s\n",msg); exit(1);}//perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	//char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) {error("ERROR opening socket"); exit(2);}

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		{error("ERROR on binding");exit(1);}
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

//loop until the server is killed
while(1)
{

	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
	if (establishedConnectionFD < 0) {error("ERROR on accept");exit(1);}

	//check to see if the client is correct
	/*char check[20];
	memset(check,'\0',sizeof(check));	
	char checkBuffer[20];
	int rcheck;*/

	// Get the message from the client and loop until it is all there
	char completeMessage[142000];
	memset(completeMessage,'\0',sizeof(completeMessage));

	int cb = 0;
	int lef = sizeof(completeMessage)-1;
	int chrc;
	while(cb < sizeof(completeMessage)-1)
	{
		chrc = recv(establishedConnectionFD,completeMessage+cb,lef,0);
		if(chrc < 0){error("server recv error");exit(1);}
		cb += chrc;
		lef -= chrc;
	}

	//set up the message and key variables
	char message[71000];
	memset(message,'\0',sizeof(message));
	
	char kcompleteMessage[71000];
	memset(kcompleteMessage,'\0',sizeof(kcompleteMessage));

	//split the whole message into two strings
	char* split = strstr(completeMessage,"**");	
	int le = strlen("**");
	//copy the message into the message variable and the key into the key variable
	strncpy(message,completeMessage,split-completeMessage);
	sprintf(kcompleteMessage,"%s",split+le);

	//remove the terminating character 
	int kterminalLocation = strstr(kcompleteMessage, "@@") - kcompleteMessage;
	kcompleteMessage[kterminalLocation] = '\0';

	//decrypt the message with the key
	int len = strlen(message);
	char encrypt[71000];
	memset(encrypt,'\0',sizeof(encrypt));
	
	int j=0;
	for(j;j<len;j++)
	{
		int mj = (int)message[j];
		if(mj == 32)// if it is a space then make it the 26th character
		{
			mj = 26;
		}
		else
		{
			mj -=65;
		}

		int kj = (int)kcompleteMessage[j];
		if(kj == 32)
		{
			kj = 26;
		}
		else
		{
			kj -=65;
		}

		int c = mj-kj;
		if(c<0)
		{
			c+=27;
		}

		if(c == 26)
		{
			c = 32;
		}
		else
		{
			c +=65;
		}
		
		encrypt[j] = (char)c;
	}

	//add terminating character
	strcat(encrypt,"@@");

	//send the decrypted message and loop until it is finished
	int charsWritten;
	int curBuffer = 0;
	int left = sizeof(encrypt)-1;
	while(curBuffer < sizeof(encrypt)-1)
	{
		charsWritten = send(establishedConnectionFD, encrypt+curBuffer,left,0);
		if(charsWritten < 0)
		{error("unable to write to sock");exit(1);}
		curBuffer += charsWritten;
		left -= charsWritten;
	}

	close(establishedConnectionFD); // Close the existing socket which is connected to the client
}
	close(listenSocketFD); // Close the listening socket
	return 0; 
}
