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
#include <fcntl.h>
#include <errno.h>
#include <netinet/tcp.h>
#include "global.h"
#include "network_common.c"

int sendmonitorcommand_datanode(char *node, char cmd);
int receivecommand_monitor()
{
	printf("Receive Monitor\n");
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
        receiveaddress.sin_port = htons(MONITOR_PORT);
        if (bind(socketb,&receiveaddress, sizeof(receiveaddress)) > 0)
	{
		printf("Bind error\n");
		exit(1);
	}
		
        // receive message
  	char buf1[1000];
	int slen = sizeof(senderaddress);
	int rc = 0;
	char host[100];
	char serv[100];
	char *cmd;
	while(1)
	{
		//printf("Waiting to receive - Monitor\n");
		memset(buf1,'\0',1000);
        	rc = recvfrom(socketb, buf1, 1000, 0, &senderaddress, &slen);
        	if (rc == 0) printf("Receive failed\n");
        	else if (rc == -1) printf("recv() failed\n");
		//printf("buffer recieved : %s",buf1);
		getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
		//printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
		cmd = strtok(buf1,",");
		//printf("REceived cmd %s\n",cmd);
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
			//sendreplicaresponse_fileexist(host,path,fret);
			printf("Fret %d\n",fret);
		}
		else if (strcmp(cmd,R_READ)==0)
		{
			printf("In R read\n");
			char *path = strtok(NULL,",");
			printf("Path : %s\n",path);
			//sendreplicaresponse_read(host,path,4096,0);
		}
		else if (strcmp(cmd,SHUTDOWN) == 0)
		{
			if (strcmp(host,monitor_namenode) == 0)
			{
				printf("Master Order : Shutdown\n");
				break;
			}
		}
		else if(strcmp(cmd,DN_COMMAND) == 0)
		{
			// Send command to dn..
			// All kinds of monitoring commands will be called here
			// Datanode name should be sent here instead of the usual path..	
			char *node = strtok(NULL,",");
			char *cmd1 = strtok(NULL,",");
			sendmonitorcommand_datanode(node,cmd1);
		}
		else if(strcmp(cmd,DISP_RESULTS) == 0)
		{
			printf("I am going to display the results I have\n");
			char *ptr = strtok(NULL,",");
			printf("\n%s\n\n",ptr);
		}
		else if(strcmp(cmd,DNLIST) == 0)
		{
			//printf("DNLIST\n");
			char *ptr = strtok(NULL," ");
			printf("Ptr value %s\n",ptr);
		}
		else if(strcmp(cmd,SENDREP) == 0)
		{
			int i_temp;

			//printf("SENDREP\n");
			char *ptr = strtok(NULL,",");
			i_temp=atoi(ptr);
			printf("Replication value set to %d\n",i_temp);
		}
	}
	shutdown(socketb,2);
	return 0;
}


// Sending Message to Datanode through COMMAND_PORT..
// Get response and display stuff to the user
int sendmonitorcommand_datanode(char *node, char cmd)
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

	int ret = sendto(socketa, &cmd, strlen(cmd), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed\n");
        else if (ret == -1) printf("send() failed\n");
        else printf("Sending done\n");
        shutdown(socketa,2);

	return 0;
} 
