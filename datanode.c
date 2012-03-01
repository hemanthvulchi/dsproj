/*
*	This file has Datanode main and other functionalities
*	This is to be executed in the namenode server
*/
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "ping.c"
#include "socket_command.c"

char *namenode;

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
        // Ping namenode and check if it is valid.
        if(sendping(argv[1]) == -1)
                return -1;
        printf("Ping success\n");
        strcpy(namenode,argv[1]);


	// Initial Conditions.. System will have a ping mechanism and can serve requests.
	pthread_t recvping_thread;
	int ping_rc = pthread_create(&recvping_thread, NULL, receiveping, NULL);
        if (ping_rc)
        {
                printf("receive ping thread create error\n");
                exit(1);
        }

	// Datanode should contact namenode and let him know that he is available.
	// Send a message to namenode.
	if (sendcommand(namenode, "", DATANODE) == -1)
        {
                //free(COMMAND_NAME);
		printf("Namenode contact failed\n");
                exit(1);
        }
	
	// Another fuctionality is to constantly wait for commands from the namenode.
	pthread_t recvcmd_thread;
	int cmd_rc = pthread_create(&recvcmd_thread, NULL, receivecommand_server, NULL);
	if (cmd_rc)
	{
		printf("Name node not to able to initiate receive comamnd thread\n");
		exit(1);
	}

	// This basically means that I should manually shutdown this namenode, or else it will always be running.
	// But what about other receivers? which one will I put as first. 
        // pthread_join(recvping_thread,NULL);
	while (1)
	{
		pthread_yield(NULL);
	}
	printf("End of main\n");
	return 0;
}
