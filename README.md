# Simple File Transfer using TCP Sockets in C
This project involves the development of a client-server application in C for transferring files from the client to the server. The communication between the client and server is based on TCP STREAM (connection-oriented) sockets, ensuring reliable data transfer.

The server takes one parameter, the port number it will listen on. To run the server, use the following command: ./ftps <port_number>

The client takes two parameters: the IP address of the server and the port number of the server. To run the client, use the following command: ./ftpc <server_ip> <server_port>

