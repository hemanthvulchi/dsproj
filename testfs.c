/*
*	This file has Namenode main and other functionalities
*	This is to be executed in the namenode server
*/
#include <stdio.h>
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
int main()
{
	//Initialize all the linked lists.. Just warm up.. :)
	int i=0;
	printf("function started\n");
	i=   dummymain();
	printf("function ended\n");

}
