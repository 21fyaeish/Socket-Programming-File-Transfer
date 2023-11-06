#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFSIZE 2097152
	
int main(int argc, char *argv[])
{
	int sd; /* socket descriptor */
	int connected_sd; /* socket descriptor */
	int rc; /* return code from recvfrom */
	struct sockaddr_in server_address;
	struct sockaddr_in from_address;
	char buffer1[100];
	char buffer2[BUFFSIZE];
	int flags = 0;
	int portNumber;
	socklen_t fromLength;
		
	if (argc < 2){
		printf ("Usage is: server <portNumber>\n");
		exit (1);
	}

	portNumber = atoi(argv[1]);
	sd = socket (AF_INET, SOCK_STREAM, 0);

	fromLength = sizeof(struct sockaddr_in);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(portNumber);
	server_address.sin_addr.s_addr = INADDR_ANY;
	
	rc = bind (sd, (struct sockaddr *)&server_address, sizeof(server_address));
	if(rc < 0){
		perror("bind");
		exit(1);
	}

	listen (sd, 5);
	connected_sd = accept (sd, (struct sockaddr *) &from_address, &fromLength);
	
	while(1){
		/* recieve size of the filename */
		int fileNameSize;
		if((rc = read(connected_sd, &fileNameSize, sizeof(int))) == 0){
			close(connected_sd);
			connected_sd = accept (sd, (struct sockaddr *) &from_address, &fromLength);
			continue;
		}
		if(rc < 0){
			perror("read");
			exit(1);
		}
		
		printf("read %d bytes to get the filenamesize \n", rc);
		printf("the size of the filename before converting is %d bytes\n", fileNameSize);
		fileNameSize = ntohl(fileNameSize);
		printf("the size of the filename after converting is %d bytes\n", fileNameSize);

		/* recieve the name of the file */
		memset(buffer1, 0, 100);
		int totalBytesFileName = 0;
		char *ptrFileName = buffer1;
		
		while(totalBytesFileName < fileNameSize){
			rc = read(connected_sd, ptrFileName, fileNameSize - totalBytesFileName);
			printf("in loop read %d filename bytes\n", rc);
			if(rc < 0){
				perror("read");
				exit(1);
			}
			totalBytesFileName += rc;
			ptrFileName += rc;
		}

		printf("recieved the following '%s'\n", buffer1);
		printf("read %d filename bytes\n", totalBytesFileName);

		/* recieve the file size */
		int fileSize;
		rc = read(connected_sd, &fileSize, sizeof(int));
		printf("read %d bytes to get the filesize\n", rc);
		printf("the size of the file before converting is %d bytes\n", fileSize);
		fileSize = ntohl(fileSize);
		printf("the size of the file after converting is %d bytes\n", fileSize);
		if(rc < 0){
			perror("read");
			exit(1);
		}


		/* recieve all bytes of file and write to disk */
		memset(buffer2, 0, BUFFSIZE);
		FILE *outputFile = fopen("new", "wb");
		int totalBytesFile = 0;

		while(totalBytesFile < fileSize){
			rc = read(connected_sd, buffer2, fileSize - totalBytesFile);
			
			if(rc < 0){
				perror("read");
				exit(1);
			}
			totalBytesFile += rc;
			rc = fwrite(buffer2, 1, rc, outputFile);
			
			if(rc < 0){
				perror("fwrite");
				exit(1);
			}
		}
		rc = fclose(outputFile);
		if(rc < 0){
			perror("closing file");
			exit(1);
		}
		
		printf("read %d file bytes\n", totalBytesFile); 	

		/* send the total bytes recieved back to client */
		int totalBytesRecieved = 0;
		totalBytesRecieved += totalBytesFile;
		int convertedTotalBytesRecieved = htonl(totalBytesRecieved);
		rc = write(connected_sd, &convertedTotalBytesRecieved, sizeof(convertedTotalBytesRecieved));
		printf("sent %d\n", totalBytesRecieved);
		if(rc < 0){
			perror("write");
			exit(1);	
		}
		
		

		

	}
	close(connected_sd);
	close(sd);

	return 0;
}

