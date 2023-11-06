#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFSIZE 2097152

int main(int argc, char *argv[])
{
	int sd;
	struct sockaddr_in server_address;
	int portNumber;
	char serverIP[29];
	int rc = 0;
	char buffer[2097152];

	if (argc < 3){
		printf ("usage is client <ipaddr> <port>\n");
		exit(1);
	}
	
	sd = socket(AF_INET, SOCK_STREAM, 0);

	portNumber = strtol(argv[2], NULL, 10);
	strcpy(serverIP, argv[1]);

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(portNumber);
	server_address.sin_addr.s_addr = inet_addr(serverIP);
	
	if(connect(sd, (struct sockaddr *)&server_address, sizeof(struct
	sockaddr_in)) < 0) {
		close(sd);
		perror("error connecting stream socket");
		exit(1);
	}
	
	/* loop asking user to input file name */
	while(1){
		FILE *inputFile;
		char inputFileName[100];

		/* ask the user for file name */
		printf("Please enter the name of the file you want to transfer: ");
		scanf("%s", inputFileName);
		if(strcmp(inputFileName, "DONE") == 0){
			break;
		}

		/* figure out the size of the file name */
		int fileNameSize = strlen(inputFileName);

		/* convert the filename to network order htonl() */
		fileNameSize = htonl(fileNameSize);

		/* send file name size to the server */
		rc = write(sd, &fileNameSize, sizeof(fileNameSize));
		printf("wrote %d bytes to send the file size\n", rc);
		if(rc < 0){
			perror("write");
			exit(1);
		}
		
		/* send file name to the server */
		int totalBytesFileName = htonl(fileNameSize) - 1;
		char *ptrFileName = inputFileName;
		rc = write(sd, ptrFileName, totalBytesFileName);
		printf("wrote %d bytes to send the file name\n", rc);
		if(rc < 0){
			perror("write");
			exit(1);
		}
		
		sleep(4);
		ptrFileName += totalBytesFileName;
		rc = write(sd, ptrFileName, 1);
		printf("wrote %d bytes to send the file name\n", rc);
		if(rc < 0){
			perror("write");
			exit(1);
		}

		/* open the file */
		if((inputFile = fopen(inputFileName, "rb")) == NULL){
			printf("Error opening the input file");
			break;
		}
		
		/* figure out the file size */
 		fseek(inputFile, 0, SEEK_END);
     		int fileSize = (int) ftell(inputFile);
		fseek(inputFile, 0, SEEK_SET);
		printf("The file size is %d\n", fileSize);
		
		/* convert that file size to network order htonl() and send to the server */
		int convertedFileSize = htonl(fileSize);
		rc = write(sd, &convertedFileSize, sizeof(convertedFileSize));
		printf("wrote %d bytes to send the file size\n", rc);
		if(rc < 0){
			perror("write");
			exit(1);
		}
		
		/* send all bytes to server */
		memset(buffer, 0, BUFFSIZE);
		int totalBytesRead = 0;
		int totalBytesWritten = 0;
		int numberOfBytes = fread(buffer, 1, 999, inputFile);
		if(numberOfBytes < 0){
			perror("reading file");
			exit(1);
		}

		while(numberOfBytes > 0){
			totalBytesRead += numberOfBytes;
			rc = write(sd, buffer, numberOfBytes);
			
			if(numberOfBytes != rc || rc < 0){
				perror("writing to server");
				exit(1);
			}
			totalBytesWritten += rc;
			numberOfBytes = fread(buffer, 1, 999, inputFile);
			if(numberOfBytes < 0){
				perror("reading file");
				exit(1);
			}

		}
		printf("read %d bytes, and wrote %d bytes\n", totalBytesRead, totalBytesWritten);

		rc = fclose(inputFile);
		if(rc < 0){
			perror("closing file");
			exit(1);
		}



		/* Wait for ACK from server */
		int totalBytesRecieved;
		rc = read(sd, &totalBytesRecieved, sizeof(int));
		totalBytesRecieved = ntohl(totalBytesRecieved);
		printf("totalBytesRecieved: %d\n", totalBytesRecieved);
		if(rc < 0){
			perror("read");
			exit(1);
		}
	}
	close(sd);
	
	return 0 ;
}

