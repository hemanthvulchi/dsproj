/*
*	This file will contain function to send and receive command and check if a server is working..
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
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "global.h"
#include "network_common.c"

#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "list.c"


// -1 - Failure 
int sendcommand(char *node, char *path, char *cmd)
{
	int socketa = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	struct sockaddr_in sendaddress;
	int slen = sizeof(sendaddress);

	char ip[100];
	if (hostname_to_ip(node, ip) == -1)
		return -1;
	sendaddress.sin_family = AF_INET;
	sendaddress.sin_port = htons(COMMAND_PORT);
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
    	if (ret == 0)
    	{
        	printf("Send failed\n");
		return -1;
    	}
    	else if (ret == -1)
    	{
        	printf("send() failed\n");
		return -1;
    	}
	printf("Sending done\n");
   	shutdown(socketa,2);
   	return 0;
}

// Ideally this function should return only when the program is going to terminate..
// It should always be running..
// May be call a function on a successful receive
int receivecommand_server()
{
	printf("Receive command server\n");
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
        receiveaddress.sin_port = htons(COMMAND_PORT);
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
	while (1)
	{
		printf("Waiting to receive\n");
        	rc = recvfrom(socketb, buf1, 100, 0, &senderaddress, &slen);
		printf("REceived\n");
        	if (rc == 0)
        		printf("Receive failed\n");
        	else if (rc == -1)
        		printf("recv() failed\n");
		char *cmd;
		cmd = strtok(buf1,",");

		getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
		printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
		//Write some breaking function here..
		// As of now, Idea is to keep waiting, if u want to break.. set a variable somewhere.. notify some node, which will ping back in acknowledgement..
		// This will make me reach this point from recvfrom.. and i will do a break here..
		if (strcmp(cmd,GETATTR)==0)
		{
			char *path;
			path = strtok(NULL,",");
			printf("User is trying to access something.. \n");
			printf("What do I do, naan enna seyya.. \n");
			//May be intimate some process.
			if(sendresponse_getattr(host, path) == -1)
			{
				continue;
			}
			printf("I am here\n");
		}
		else if (strcmp(cmd,DATANODE)==0)
		{
			//Now add it to the linked list in namenode..
			char _ip[100];
        		if (hostname_to_ip(host, _ip) == -1)
                		return -1;
			datanode_insert(host,_ip);
			datanode_display();
		}
		else if (strcmp(cmd,ACCESS)==0)
		{
			char *path;
                        path = strtok(NULL,",");
			printf("Path : %s\n",path);
			char *mask = strtok(NULL,",");
			if (sendresponse_access(host,path,atoi(mask)) == -1)
				continue;
		}
		else if(strcmp(cmd,READDIR)==0)
		{
			printf("In readdir\n");
			char *path;
			path = strtok(NULL, ",");
			if (sendresponse_readdir(host,path) == -1)
				continue;
		}
		else if(strcmp(cmd,OPEN)==0)
		{
			printf("In open\n");
			char *path = strtok(NULL,",");
			printf("Path : %s\n",path);
			char *flags = strtok(NULL,",");
			printf("Flags %s\n",flags);
			if (sendresponse_open(host,path,atoi(flags)) == -1)
				continue;
		}
		else
		{
			printf("Unknown command %s\n",cmd);
		}
	}
	shutdown(socketb,2);
	return 0;
}

int sendresponse_open(char *node, char *path, int flags)
{
        printf("Send response access %s %d\n", path, flags);
        int socketa = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        struct sockaddr_in sendaddress;
        int slen = sizeof(sendaddress);

        char ip[100];
        if (hostname_to_ip(node, ip) == -1)
                return -1;
        sendaddress.sin_family = AF_INET;
        sendaddress.sin_port = htons(RESPONSE_PORT);
        if (inet_aton(ip, &sendaddress.sin_addr)==0) {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }

        int ret;
        int res;
        res = open(path, flags);
        if (res == -1)
        {
                res = -errno;
                ret = sendto(socketa, &res, sizeof(res), 0, &sendaddress, slen);
        }
        else
	{
                ret = sendto(socketa, &res, sizeof(res), 0, &sendaddress, slen);
		close(res);
	}
        if (ret == 0)
        {
                printf("Send failed\n");
                return -1;
        }
        else if (ret == -1)
        {
                printf("send() failed\n");
                return -1;
        }
        printf("Sending response done\n");

	close(socketa);
        return 0;
}

int sendresponse_access(char *node, char *path, int mask)
{
	printf("Send response access %s\n", path);
        int socketa = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        struct sockaddr_in sendaddress;
        int slen = sizeof(sendaddress);

        char ip[100];
        if (hostname_to_ip(node, ip) == -1)
                return -1;
        sendaddress.sin_family = AF_INET;
        sendaddress.sin_port = htons(RESPONSE_PORT);
        if (inet_aton(ip, &sendaddress.sin_addr)==0) {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }

	int ret;
	int res;
        res = access(path, mask);
        if (res == -1)
	{
		res = -errno;
		ret = sendto(socketa, &res, sizeof(res), 0, &sendaddress, slen);
                return -errno;	
	}
	else
	{
		ret = sendto(socketa, &res, sizeof(res), 0, &sendaddress, slen);
	}
	if (ret == 0)
        {
                printf("Send failed\n");
                return -1;
        }
        else if (ret == -1)
        {
                printf("send() failed\n");
                return -1;
        }
        printf("Sending response done\n");

        close(socketa);
        return 0;
}

int sendresponse_readdir(char *node, char *path)
{
	printf("Send response readdir %s\n", path);
	int socketa = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        struct sockaddr_in sendaddress;
        int slen = sizeof(sendaddress);

        char ip[100];
        if (hostname_to_ip(node, ip) == -1)
                return -1;
        sendaddress.sin_family = AF_INET;
        sendaddress.sin_port = htons(RESPONSE_PORT);
        if (inet_aton(ip, &sendaddress.sin_addr)==0) {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }

	int count = 0;
	DIR *dp;
	struct dirent *de;
	dp = opendir(path);
	//dp = opendir("/tmp/shyam-fuse");
	if (dp == NULL)
		return -errno;

	char listfiles[10000];
	memset(listfiles,'\0',10000);
	strcpy(listfiles,"");
	while ((de = readdir(dp)) != NULL)
	{
		printf("d_name %s\n",de->d_name);
		printf("d_type %d\n",de->d_type);
		printf("d_ino %d\n",de->d_ino);
		strcat(listfiles,de->d_name);
		strcat(listfiles,",");
		char buffer[30];
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",de->d_type);
		strcat(listfiles, buffer);
		strcat(listfiles, ",");
		//strcpy(buffer,"");
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",de->d_ino);
		strcat(listfiles, buffer);
		strcat(listfiles,",");
	}
	int ret = sendto(socketa, listfiles, strlen(listfiles), 0, &sendaddress, slen);
        if (ret == 0)
        {
                printf("Send failed\n");
                return -1;
        }
        else if (ret == -1)
        {
                printf("send() failed\n");
                return -1;
        }
        printf("Sending response done\n");
	
        close(socketa);
        return 0;
}

int sendresponse_getattr(char *node, char *path)
{
	printf("Send response getattr to %s\n",node);
	printf("Requested path %s\n",path);
	int socketa = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        struct sockaddr_in sendaddress;
        int slen = sizeof(sendaddress);

        char ip[100];
        if (hostname_to_ip(node, ip) == -1)
                return -1;
        sendaddress.sin_family = AF_INET;
        sendaddress.sin_port = htons(RESPONSE_PORT);
        if (inet_aton(ip, &sendaddress.sin_addr)==0) {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }

	/*if (strcmp(path,"/hello.txt") == 0)
		strcpy(path,"/tmp/shyam-fuse/hello.txt");*/

        // this is code to send message
	struct stat st;
	if (stat(path, &st) == -1)
	{
		printf("error doing stat\n");
	}
	// I should load contents in st before I send..
        int ret = sendto(socketa, &st, sizeof(struct stat), 0, &sendaddress, slen);
        if (ret == 0)
        {
                printf("Send failed\n");
                return -1;
        }
        else if (ret == -1)
        {
                printf("send() failed\n");
                return -1;
        }
        printf("Sending response done\n");
        //shutdown(socketa,2);
	close(socketa);
        return 0;
}

int receiveresponse_client()
{
	printf("Receive command server\n");
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
        receiveaddress.sin_port = htons(RESPONSE_PORT);
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
        if (strcmp(COMMAND_NAME,GETATTR)==0)
	{
        	st_getattr = malloc (sizeof(struct stat));
        	printf("Waiting to receive Getattr\n");
        	rc = recvfrom(socketb, st_getattr, sizeof(struct stat), 0, &senderaddress, &slen);
        	if (rc == 0)
        		printf("Receive failed\n");
        	else if (rc == -1)
        		printf("recv() failed\n");
		printf("uid : %d\n",st_getattr->st_uid);
        	getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
        	printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
                //printf("Received packet from %s : %d bytes\nData: %s\n\n",inet_ntoa(senderaddress.sin_addr), ntohs(senderaddress.sin_port), buf1);
	}
	else if (strcmp(COMMAND_NAME, READDIR)==0)
	{
		memset(readdir_buf,'\0',10000);
		rc = recvfrom(socketb, &readdir_buf, 10000, 0, &senderaddress, &slen);
                if (rc == 0)
                        printf("Receive failed\n");
                else if (rc == -1)
                        printf("recv() failed\n");
		printf("Count : %s\n", readdir_buf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if( strcmp(COMMAND_NAME, ACCESS) == 0)
	{
		rc = recvfrom(socketb, &access_return, sizeof(int), 0, &senderaddress, &slen);
		if (rc == 0)
                        printf("Receive failed\n");
                else if (rc == -1)
                        printf("recv() failed\n");
                printf("Acess Return : %d\n", access_return);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, OPEN) == 0)
	{
		rc = recvfrom(socketb, &open_return, sizeof(int), 0, &senderaddress, &slen);
                if (rc == 0)
                        printf("Receive failed\n");
                else if (rc == -1)
                        printf("recv() failed\n");
                printf("Open Return : %d\n",open_return);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
        close(socketb);
	return 0;
}
