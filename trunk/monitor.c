/*
* This file will contain the functionality for performing monitoring
* Author : Shyam (shyam2347@gmail.com)
* 
*/
#include <stdio.h>
#include <pthread.h>
#include "global.h"
#include "ping.c"
#include "monitor_sockets.c"
#include "socket_command.c"

char *namenode;
void helpme()
{
	printf("\nList of Commands\n");
	printf("LHS = Commands, RHS = Action\n");
	printf("help me => help menu\n");
	printf("dn list => datanode list\n");
	printf("shut down => shut down\n");
	printf("replica 5 => Change replication to 5\n");
}

void dnlist()
{
	//printf("Namenode : %s\n",namenode);
	int socketa = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        struct sockaddr_in sendaddress;
        int slen = sizeof(sendaddress);

        char ip[100];
        if (hostname_to_ip(namenode, ip) == -1)
                return -1;
        sendaddress.sin_family = AF_INET;
        sendaddress.sin_port = htons(MONITOR_PORT);
        if (inet_aton(ip, &sendaddress.sin_addr)==0) {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }
	char cmd[100];
	strcpy(cmd,DNLIST);
        int ret = sendto(socketa, &cmd, strlen(cmd), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed\n");
        else if (ret == -1) printf("send() failed\n");
        else printf("Sending done\n");
        shutdown(socketa,2);

        return 0;
}

void replica(int count)
{
	printf("Replica ->Namenode : %s count:%d\n",namenode,count);
	int socketa = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        struct sockaddr_in sendaddress;
        int slen = sizeof(sendaddress);

        char ip[100];
        if (hostname_to_ip(namenode, ip) == -1)
                return -1;
        sendaddress.sin_family = AF_INET;
        sendaddress.sin_port = htons(MONITOR_PORT);
        if (inet_aton(ip, &sendaddress.sin_addr)==0) {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }
	char cmd[100];
	char buf[100];
	memset(cmd,'\0',100);
	memset(buf,'\0',100);
	strcpy(cmd,SENDREP);
	snprintf(buf,10,"%d",count);
	strcat(cmd,",");
	strcat(cmd,buf);
        int ret = sendto(socketa, &cmd, strlen(cmd), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed\n");
        else if (ret == -1) printf("send() failed\n");
        else printf("Sending done\n");
        shutdown(socketa,2);

        return 0;
}


int main(int argc, char *argv[])
{
	//USAGE
        if(argc < 2)
        {
                printf("\nUSAGE : %s <namenode-name>\n\n",argv[0]);
                exit(1);
        }

	// Check namenode details here - return -1 for error..
        namenode = malloc (strlen(argv[1])*sizeof(char)+1);
	strcpy(namenode,argv[1]);
	memset(monitor_namenode,'\0',SERV_PATH);
	strcpy(monitor_namenode,namenode);
	printf("starting to ping the server \n");
	pthread_t recvping_thread;
	int ping_rc = pthread_create(&recvping_thread, NULL, receiveping, NULL);
        if (ping_rc)
        {
                printf("receive ping thread create error\n");
                exit(1);
        }

	//printf("Namenode %s\n",namenode);
        // Ping namenode and check if it is valid.
        if(sendping(argv[1]) == -1)
                return -1;
	int counter = 0;
	while(pingsuccess != 1)
	{
		usleep(1);
		counter++;
		if(counter > 100)
		{
			printf("Namenode not reachable\n");
			exit(1);
		}
	}
	//printf("After success %d\n",counter);

	pthread_t recvcmd_thread;
	int cmd_rc = pthread_create(&recvcmd_thread, NULL, receivecommand_monitor, NULL);
	if (cmd_rc)
	{
		printf("Monitor tool not able to initiate receive comamnd thread\n");
		exit(1);
	}

	// Send a message to namenode.
        if (sendcommand(namenode, "", MONITOR) == -1)
        {
                printf("Namenode contact failed\n");
                exit(1);
        }
	usleep(10);

	printf("\nEnter your command <help me for help>\n");
	char first[20];
	char second[20];
	while(1)
	{
		memset(first,'\0',20);
		memset(second,'\0',20);
		printf("['shut down' to quit]> ");	
		scanf("%s %s",&first,&second);
		if( (strcmp(first,"help")==0) && (strcmp(second,"me")==0) )
			helpme();
		else if( (strcmp(first,"dn")==0) && (strcmp(second,"list")==0) )
			dnlist();
		else if( (strcmp(first,"shut")==0) && (strcmp(second,"down")==0) )
			break;
		else if(strcmp(first,"replica")==0)
		{
			int count = atoi(second);
			printf("Count : %d\n",count);
			replica(count);
		}
		else
			printf("\nError : Invalid Command\n");
	}

	//pthread_join(recvcmd_thread, NULL);
	return 0;
}
