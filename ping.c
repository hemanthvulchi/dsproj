/*
*	This file will contain function to ping and check if a server is working..
*	I will try to write it in a generic way such that any function can use it..
*	There will be 2 threads in every node..
*	1 for send, another for receive..
*	Key:
*	Send will send IP address..
*	Receive will accept from any IP address..
*	This is how I am going to implement
*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "network_common.c"

// -1 - Failure 
int sendping(char *node)
{
	int socketa = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	struct sockaddr_in sendaddress;
	int slen = sizeof(sendaddress);

	char ip[100];
	if (hostname_to_ip(node, ip) == -1)
		return -1;
	sendaddress.sin_family = AF_INET;
	sendaddress.sin_port = htons(PING_PORT);
	if (inet_aton(ip, &sendaddress.sin_addr)==0) {
        	fprintf(stderr, "inet_aton() failed\n");
          	exit(1);
        }

	// this is code to send message
	char buf[15] = "ping"; 
	int ret = sendto(socketa, buf, strlen(buf), 0, &sendaddress, slen);
    	if (ret == 0)
    	{
        	printf("Send failed\n");
    	}
    	else if (ret == -1)
    	{
        	printf("send() failed\n");
    	}
   	shutdown(socketa,2);
   	return 0;
}

// Ideally this function should return only when the program is going to terminate..
// It should always be running..
// May be call a function on a successful receive
int receiveping()
{
	printf("REceive Ping\n");
	struct sockaddr_in receiveaddress, senderaddress;
	
	// create Non-Blocking socket to send message
        int socketb = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketb == -1)
	{
		printf("Socket Error\n");
		return -1;
	}

        // set address to local address
        unsigned long lNB = 1; // Non-Blocking socket
        ioctl(socketb, 0, &lNB);
        receiveaddress.sin_addr.s_addr = htonl(INADDR_ANY);
        receiveaddress.sin_family = AF_INET;
        receiveaddress.sin_port = htons(PING_PORT);
        if (bind(socketb,&receiveaddress, sizeof(receiveaddress)) > 0)
	{
		printf("Bind error\n");
		exit(1);
	}

        // receive message
        char buf1[100];
	memset (buf1,'\0',100);
	int slen = sizeof(senderaddress);
	int rc = 0;
	char host[100];
	char serv[100];
	while (1)
	{
        	rc = recvfrom(socketb, buf1, 100, 0, &senderaddress, &slen);
        	if (rc == 0)
        		printf("Receive failed\n");
        	else if (rc == -1)
        		printf("recv() failed\n");

		//Write some breaking function here..
		// As of now, Idea is to keep waiting, if u want to break.. set a variable somewhere.. notify some node, which will ping back in acknowledgement..
		// This will make me reach this point from recvfrom.. and i will do a break here..
		if (strcmp(buf1,"ping")==0)
		{
			getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
			printf("Ping received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
			//proper ping, call some function here..
			//May be intimate some process.
		}
        	//printf("Received packet from %s : %d bytes\nData: %s\n\n",inet_ntoa(senderaddress.sin_addr), ntohs(senderaddress.sin_port), buf1);
	}
	shutdown(socketb,2);
	return 0;
}
