/*
*	Key:
*	Send will send IP address..
*	Receive will accept from any IP address..
*	This is how I am going to implement
*/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "global.h"
#include "network_common.c"

// -1 - Failure 
int sendcommand_replica(char *node, char *cmd, char *path)
{
	int socketa = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	struct sockaddr_in sendaddress;
	int slen = sizeof(sendaddress);

	char ip[100];
	if (hostname_to_ip(node, ip) == -1)
		return -1;
	sendaddress.sin_family = AF_INET;
	sendaddress.sin_port = htons(REPLICATION_PORT);
	if (inet_aton(ip, &sendaddress.sin_addr)==0) {
        	fprintf(stderr, "inet_aton() failed\n");
          	exit(1);
        }

	// this is code to send message
	char buf[1000];
	memset(buf,'\0',1000);
	strcpy(buf,"");
	strcat(buf,cmd);
	strcat(buf,",");
	strcat(buf,path);

	int ret = sendto(socketa, buf, strlen(buf), 0, &sendaddress, slen);
    	if (ret == 0) printf("Send failed\n");
    	else if (ret == -1) printf("send() failed\n");
   	shutdown(socketa,2);
   	return 0;
}

// 0 if file exists, 1 if file does not exist
int fexist(char *filename) 
{
  struct stat buffer;
  if ( stat(filename, &buffer) ) return 1;
  return 0;
}

int receivecommand_replica()
{
	printf("Receive Replica\n");
	struct sockaddr_in receiveaddress, senderaddress;
	char *dpath=NULL;

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
        receiveaddress.sin_port = htons(REPLICATION_PORT);
        if (bind(socketb,&receiveaddress, sizeof(receiveaddress)) > 0)
	{
		printf("Bind error\n");
		exit(1);
	}

        // receive message
  	char buf1[1000];
	memset (buf1,'\0',1000);
	int slen = sizeof(senderaddress);
	int rc = 0;
	char host[100];
	char serv[100];
	char *cmd;
	while(1)
	{
		printf("Waiting to receive - Replica\n");
		memset(buf1,'\0',1000);
        	rc = recvfrom(socketb, buf1, 1000, 0, &senderaddress, &slen);
        	if (rc == 0) printf("Receive failed\n");
        	else if (rc == -1) printf("recv() failed\n");
		getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
		printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
		cmd = strtok(buf1,",");
		printf("REceived cmd %s\n",cmd);
		if (strcmp(cmd,R_FILEEXIST)==0)
		{
			printf("In Fileexists\n");
			char *path = strtok(NULL,",");
			printf("Path : %s\n",path);
			int fret = fexist(path);	
			if (fret == 0)
			{
				printf("File exists\n");
			}
			sendreplicaresponse_fileexist(host,path,fret);
			printf("Fret %d\n",fret);
		}
	}
	shutdown(socketb,2);
	return 0;
}

int sendreplicaresponse_fileexist(char *node, char *path, int fret)
{
	printf("Send response readdir %s\n", path);
	int socketa = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        struct sockaddr_in sendaddress;
        int slen = sizeof(sendaddress);

        char ip[100];
        if (hostname_to_ip(node, ip) == -1)
                return -1;
	printf("After hostname\n");
        sendaddress.sin_family = AF_INET;
        sendaddress.sin_port = htons(REPLICATION_PORT);
        if (inet_aton(ip, &sendaddress.sin_addr)==0) {
                printf(stderr, "inet_aton() failed\n");
                exit(1);
        }
	char fexistbuf[100];
	memset(fexistbuf,'\0',100);
	strcpy(fexistbuf,"");

	char buffer[30];
	memset(buffer,'\0',30);
	snprintf(buffer, 10,"%d",fret);
	strcat(fexistbuf, buffer);
	strcat(fexistbuf, ",");
	int ret = sendto(socketa, fexistbuf, strlen(fexistbuf), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed\n");
        else if (ret == -1) printf("send() failed\n");
        else printf("Sending response done\n");
	
        close(socketa);
        return 0;

}

int receiveresponse_replica()
{
	printf("Receive command Replica\n");
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
        receiveaddress.sin_port = htons(REPLICATION_PORT);
        if (bind(socketb,&receiveaddress, sizeof(receiveaddress)) > 0)
        {
                printf("Bind error\n");
                exit(1);
        }
	// receive message
        int slen = sizeof(senderaddress);
        int rc = 0;
        char host[100];
        char serv[100];
	printf("Before IF\n");
	printf("R_COMMAND_NAME %s\n",R_COMMAND_NAME);
	printf("R_FILEEXIST %s\n",R_FILEEXIST);
        if (strcmp(R_COMMAND_NAME,R_FILEEXIST)==0)
	{
		memset(r_fexist_buf,'\0',100);
        	printf("Waiting to receive Getattr\n");
        	rc = recvfrom(socketb, &r_fexist_buf, 100, 0, &senderaddress, &slen);
        	if (rc == 0) printf("Receive failed\n");
        	else if (rc == -1) printf("recv() failed\n");
		printf("File exist buf : %s\n", r_fexist_buf);
        	getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
        	printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	return 0;
}
