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

int main(int argc, char *argv[])
{
	pthread_t recvping_thread;
	int ping_rc = pthread_create(&recvping_thread, NULL, receiveping, NULL);

        if (ping_rc)
        {
                printf("receive ping thread create error\n");
                exit(1);
        }
	pthread_t recvcmd_thread;
	int cmd_rc = pthread_create(&recvcmd_thread, NULL, receivecommand_namenode, NULL);
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
	return 0;

}
