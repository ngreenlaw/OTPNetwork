/*Nathan Greenlaw
 * CS 344
 * Program 4: otp_dec decrypt text with a given key by sending it to otp_dec_d*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/ioctl.h>

void error(const char *msg) { fprintf(stderr,"%s\n",msg);exit(0);}//perror(msg); exit(0); } // Error function used for reporting issues

//returns the size of a file
int fSize(char* f)
{
	FILE *fp = fopen(f,"rb");
	if(fp != NULL)
	{
		//get the length of the file
		fseek(fp,0,SEEK_END);
		int lengthOfFile = ftell(fp);

		fclose(fp);
		return lengthOfFile;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead, kcharsWritten;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	//char buffer[256];
    
	if (argc < 4) { fprintf(stderr,"USAGE: %s text key port\n", argv[0]); exit(0); } // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)serverHostInfo->h_addr, (char*)&serverAddress.sin_addr.s_addr, serverHostInfo->h_length); // Copy in the address


//get the file size to set up buffer
	int fileSize = fSize(argv[1]);
	if(fileSize == 0) {error("File either does not exist or contains nothing");exit(1);}
	fileSize +=2; //for the terminating characters
	
	char buffer[71000];
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
	
	//get the contents of the file and store them into buffer
	FILE *fp = fopen(argv[1],"rb");
	if(fp != NULL)
	{
	//get the length of the file
	fseek(fp,0,SEEK_END);
	int lengthOfFile = ftell(fp);

	//read the file
	fseek(fp,0,SEEK_SET); // reset to beginning of file
	fread(buffer,lengthOfFile,1,fp);

	// strip newline and error check the input file
	int i=0;
	for(i;i<lengthOfFile;i++)
	{
		int ascii = (int)buffer[i];
		if(buffer[i] == '\n')
		{
			buffer[i] = '*';
			strcat(buffer,"*"); 
		}
		else
		{
			if((ascii < 65 || ascii > 90) && ascii != 32 && ascii != 64)
			{
				error("Bad Character in file\n");
				fclose(fp);
				error("Bad input in the files");exit(1);
			}
		}
	}
	
	fclose(fp);
	}
	else{error("File does not exist\n");exit(1);}

	//get the file size to set up buffer
	int kfileSize = fSize(argv[2]);
	if(kfileSize == 0) {error("File either does not exist or contains nothing");exit(1);}
	kfileSize +=2; //for the terminating characters
	
	if(kfileSize+1 < fileSize)
	{
		error("Key length too short."); exit(1);
	}


	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) {error("CLIENT: ERROR opening socket");exit(2);}

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		{error("CLIENT: ERROR connecting to server");exit(2);}

	//check if this the correct server
	/*int correct;
	char code[20];
	memset(code,'\0',sizeof(code));
	strcpy(code,"b@@");
*/
	char kbuffer[71000];
	memset(kbuffer, '\0', sizeof(kbuffer)); // Clear out the buffer array

	//get the contents of the file and store them into buffer
	FILE *kfp = fopen(argv[2],"rb");
	if(kfp != NULL)
	{
	//get the length of the file
	fseek(kfp,0,SEEK_END);
	int klengthOfFile = ftell(kfp);

	fseek(kfp,0,SEEK_SET); // reset to beginning of file
	fread(kbuffer,fileSize-3,1,kfp);

	
	// strip newline and error check the key file
	int i=0;
	int replace = 0;
	for(i;i<fileSize-3;i++)
	{
		int kascii = (int)kbuffer[i];
		if(kbuffer[i] == '\n')
		{
			kbuffer[i] = '@';
			strcat(kbuffer,"@"); 
			replace = 1;
		}
		else
		{
			if((kascii < 65 || kascii > 90) && kascii != 32 && kascii != 64)
			{
				error("Bad Character in file\n");
				fclose(kfp);
				error("Bad input in the files\n");exit(1);
			}
		}
	}
	if(replace == 0)
	{
		kbuffer[fileSize-3] = '@';
		strcat(kbuffer,"@"); 
	}
	fclose(kfp);
	}
	else{error("File does not exist\n");exit(1);}

	char completeMessage[142000];
	memset(completeMessage, '\0', sizeof(completeMessage)); // Clear out the buffer array
	
	//create the entire message of key and text
	strcat(completeMessage,buffer);
	strcat(completeMessage,kbuffer);

	//send the text and key and loop until it is finished
	int chrs;
	int cBuffer = 0;
	int lefts = sizeof(completeMessage)-1;
	while(cBuffer < sizeof(completeMessage)-1)
	{
		chrs = send(socketFD, completeMessage+cBuffer,lefts,0);
		cBuffer += chrs;
		lefts -= chrs;
	}

	// loop until all of the decrypted text is recieved and then print it	
	char echeck[71000];
	memset(echeck,'\0',sizeof(echeck));	

	int curBuffer = 0;
	int left = sizeof(echeck)-1;
	int chr;
	while(curBuffer < sizeof(echeck)-1)
	{
		chr = recv(socketFD,echeck+curBuffer,left,0);
		if(chr < 0){error("Client recv error");exit(1);}
		curBuffer += chr;
		left -= chr;
	}
	
	//remove the terminating character
	int terminalLocation = strstr(echeck, "@@") - echeck;
	echeck[terminalLocation] = '\0';

	//display the decrypted text
	printf("%s\n",echeck);
	
	close(socketFD); // Close the socket
	return 0;
}
