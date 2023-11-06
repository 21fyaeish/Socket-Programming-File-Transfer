#Define targets
all: ftps ftpc

#Compile server
ftps: ftps.c
	gcc -o ftps ftps.c

#Compile client
ftpc: ftpc.c
	gcc -o ftpc ftpc.c

#Clean up generated files
clean:
	rm -f ftps ftpc
