/*
*
*	This file will contain all the functionalities for creating and managing replicas
*	Author : Shyam (shyam2347@gmail.com)
*	Date : March 3rd, 2012
*
*/
#include "global.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "network_common.c"
#include "replica_sockets.c"
#include "ping.c"
// -1 if hostname_to_ip fails
// 1 if success
// 0 if file exists..
int check_filepresent(char *path, char *repdn)
{
	char ip[100];
	if (hostname_to_ip(repdn,ip) == -1)
		return -1;
	printf("host %s Ip : %s\n",repdn,ip);

	// I have to send the path from here to the namenode..
        // Get back the list of files..
        R_COMMAND_NAME = malloc (1+sizeof(char)*strlen(R_FILEEXIST));
        strcpy(R_COMMAND_NAME, R_FILEEXIST);
        pthread_t recvcmd_thread;
        int cmd_rc = 0;
        pthread_create(&recvcmd_thread, NULL, receiveresponse_replica, NULL);
        if (cmd_rc)
        {
                printf("Name node not to able to initiate receive comamnd thread\n");
                free(R_COMMAND_NAME);
                exit(1);
        }
	if (sendcommand_replica(repdn,R_FILEEXIST,path) == -1)
	{
		printf("Send command failed\n");
                pthread_kill(recvcmd_thread,0);
                free(R_COMMAND_NAME);
                return -1;
	}
	printf("Waiting for pthread join\n");
        pthread_join(recvcmd_thread, NULL);
        free(R_COMMAND_NAME);
        pthread_kill(recvcmd_thread,0);

	char *ptr = strtok(r_fexist_buf,",");
	int res = atoi(ptr);
	if (res == 0)
		return 1;
	else if (res == 1)
		return 0;
	else
	{
		printf("This is an error\n");
		return 0;
	}
}

// Path is the absolute filename
// Primdn is the primary datanode name
// repdn is the datanode in which the file has to be replicated
// -1 - error..
int create_replica(char *path, char *primdn, char *repdn)
{
	//Check if replica is alive..

	//Check if the path is dir..
	int ret;
	ret = isDir(path);
	if (ret == 1)
	{
		printf("Is directory\n");
	}
	//Check if the given path is not in repdn..
	//If present, the it shd need a sync not create..
	ret = check_filepresent(path, repdn);
	if (ret == -1)
	{
		printf("Return error\n");
		return -1;
	}
	printf("File present checked..\n");
	if(ret == 1)
	{
		printf("File is present.. you should have done sync\n");
	}
	else
	{
		printf("File not present.. So, I should do replication\n");
	}
	// Get the file contents from primdn..
	// I have to send the path from here to the namenode..
        // Get back the list of files..
        R_COMMAND_NAME = malloc (1+sizeof(char)*strlen(R_READ));
        strcpy(R_COMMAND_NAME, R_READ);
        pthread_t recvcmd_thread;
        int cmd_rc = 0;
        pthread_create(&recvcmd_thread, NULL, receiveresponse_replica, NULL);
        if (cmd_rc)
        {
                printf("Name node not to able to initiate receive comamnd thread\n");
                free(R_COMMAND_NAME);
                exit(1);
        }
	datanoderesponse_success = 0;
        if (sendcommand_replica(primdn,R_READ,path) == -1)
        {
                printf("Send command failed\n");
                pthread_kill(recvcmd_thread,0);
                free(R_COMMAND_NAME);
                return -1;
        }
        printf("Waiting for pthread join\n");
	int counter = 0;
	/*while(datanoderesponse_success != 1)
	{
		usleep(10);
		counter++;
		if(counter > 1000)
		{
			printf("Enough waiting R\n");
			pthread_kill(recvcmd_thread,0);
        		free(R_COMMAND_NAME);
			return -1;
		}
	}*/
        pthread_join(recvcmd_thread, NULL);
        free(R_COMMAND_NAME);
        pthread_kill(recvcmd_thread,0);

	char *ptr = strtok(r_read_buf,",");
	int res = atoi(ptr);
	ptr = strtok(NULL,",");
	errno = atoi(ptr);
	printf("res : %d\n", res);
	printf("errno : %d\n", errno);
	if (res == -1)
	{
		return -errno;
	}
	ptr = strtok(NULL,",");
	printf("buf : %s\n",ptr);
	char buf[4096];
	memset(buf,'\0',4096);
	if(ptr != NULL) strcpy(buf,ptr);
	printf("Buffer content : %s\n",buf);
	pthread_kill(recvcmd_thread,0);

	// Write the contents in repdn..
	// I have to send the path from here to the namenode..
        // Get back the list of files..
        R_COMMAND_NAME = malloc (1+sizeof(char)*strlen(R_WRITE));
        strcpy(R_COMMAND_NAME, R_WRITE);
        recvcmd_thread;
        cmd_rc = 0;
        pthread_create(&recvcmd_thread, NULL, receiveresponse_replica, NULL);
        if (cmd_rc)
        {
                printf("Name node not to able to initiate receive comamnd thread\n");
                free(R_COMMAND_NAME);
                exit(1);
        }
	char my_path[4196];
	memset(my_path,'\0',4196);
	strcpy(my_path,"");
	strcat(my_path,path);
	strcat(my_path,",");
	strcat(my_path,buf);
	printf("Mypath, %s\n",my_path);
	datanoderesponse_success = 0;
        if (sendcommand_replica(repdn, R_WRITE,my_path) == -1)
        {
                printf("Send command failed\n");
                pthread_kill(recvcmd_thread,0);
                free(R_COMMAND_NAME);
                return -1;
        }
        printf("Waiting for pthread join\n");
	counter = 0;
	while(datanoderesponse_success != 1)
	{
		usleep(10);
		counter++;
		if(counter > 100)
		{
			printf("Enough waiting\n");
			pthread_kill(recvcmd_thread,0);
        		free(R_COMMAND_NAME);
			return -1;
		}
	}
        //pthread_join(recvcmd_thread, NULL);
        free(R_COMMAND_NAME);
	ptr = strtok(r_write_buf,",");
        res = atoi(ptr);
        ptr = strtok(NULL,",");
        errno = atoi(ptr);
        if (res == -1)
                res = -errno;
	pthread_kill(recvcmd_thread,0);

}
