/*
*	This file has Namenode main and other functionalities
*	This is to be executed in the namenode server
*/
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "ping.c"
#include "list.c"
#include "socket_command.c"
#include "dd.h"
#include "dd.c"


// Its not already init, its the big init..
// All of heamnath's code will go in here..
void init()
{
	//Initialize all the linked lists.. Just warm up.. :)

//	initfilelist();
	initdatanodelist();
    ninitialize(); 

	//Load data from files, this only applies during recovery..
}

int monitorcommand_namenode()
{
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
        receiveaddress.sin_port = htons(MONITOR_PORT);
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
	char recvbuf[1000];
	while(1)
	{
		memset(host,'\0',100);
		memset(serv,'\0',100);
		memset(recvbuf,'\0',1000);
		rc = recvfrom(socketb, &recvbuf, 1000, 0, &senderaddress, &slen);
		if (rc == 0) printf("Receive failed\n");
       		else if (rc == -1) printf("recv() failed\n");
		printf("Recv buf : %s\n", recvbuf);
		getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
		char *cmd = strtok(recvbuf,",");
		if (strcmp(cmd,DNLIST)==0)
		{
			printf("I am in DN List\n");
			char dnbuf[10000];
			memset(dnbuf,'\0',10000);
			datanode_sendlist(&dnbuf);
			senddnlist(host,dnbuf);
		}
		if (strcmp(cmd,SENDREP)==0)
		{
			int i_temp;
			printf("SENDREP\n");
			char *ptr ;
			ptr=strtok(NULL,",");
			//printf("This is done\n");
			i_temp=atoi(ptr);
			//printf("done too itemp=%d \n",i_temp);
				replication_count = i_temp;
			printf("Replication value set to %d\n",replication_count);
			printf("sending ack \n");
			sendAckRep(host);
		}

	}
}
int sendAckRep(char *node)
{
	int socketa = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        struct sockaddr_in sendaddress;
        int slen = sizeof(sendaddress);

        char ip[100];
        if (hostname_to_ip(node, ip) == -1)
                return -1;
        sendaddress.sin_family = AF_INET;
        sendaddress.sin_port = htons(MONITOR_PORT);
        if (inet_aton(ip, &sendaddress.sin_addr)==0) {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }
	int i=replication_count;
	char buf[10];
	memset(buf,'\0',10);
	snprintf(buf,10,"%d",i);
        char cmd[20];
	memset(cmd,'\0',10);
        strcpy(cmd,SENDREP);
	strcat(cmd,",");
	strcat(cmd,buf);
	//printf("buffer :%s",cmd);
        int ret = sendto(socketa, &cmd, strlen(cmd), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed\n");
        else if (ret == -1) printf("send() failed\n");
        else printf("Sending done\n");
        shutdown(socketa,2);
        
        return 0;
}
int senddnlist(char *node, char *dnbuf)
{
	int socketa = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        struct sockaddr_in sendaddress;
        int slen = sizeof(sendaddress);

        char ip[100];
        if (hostname_to_ip(node, ip) == -1)
                return -1;
        sendaddress.sin_family = AF_INET;
        sendaddress.sin_port = htons(MONITOR_PORT);
        if (inet_aton(ip, &sendaddress.sin_addr)==0) {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }
        char cmd[10000];
        strcpy(cmd,DNLIST);
	strcat(cmd,",");
	strcat(cmd,dnbuf);
        int ret = sendto(socketa, &cmd, strlen(cmd), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed\n");
        else if (ret == -1) printf("send() failed\n");
        else printf("Sending done\n");
        shutdown(socketa,2);
        
        return 0;
}

int main(int argc, char *argv[])
{
	pthread_t recvping_thread;
	int ping_rc = pthread_create(&recvping_thread, NULL, receiveping, NULL);
	
        if (ping_rc)
        {
                printf("receive ping thread create error\n");
                exit(1);
        }
	init();
	pthread_t recvcmd_thread;
	int cmd_rc = pthread_create(&recvcmd_thread, NULL, receivecommand_namenode, NULL);
	if (cmd_rc)
	{
		printf("Name node not to able to initiate receive comamnd thread\n");
		exit(1);
	}

	pthread_t monitorcmd_thread;
        cmd_rc = pthread_create(&recvcmd_thread, NULL, monitorcommand_namenode, NULL);
        if (cmd_rc)
        {
                printf("Name node not to able to initiate receive Monitor thread\n");
                exit(1);
        }


	// This basically means that I should manually shutdown this namenode, or else it will always be running.
	// But what about other receivers? which one will I put as first. 
        // pthread_join(recvping_thread,NULL);
	while (1)
	{
		pthread_yield(NULL);
	}
	return 0;
}
