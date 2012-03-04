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
#include "dd.h"
#include "network_common.c"

#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include "list.c"
#include "dd.c"

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
	char *cmd;
	while (1)
	{
		printf("Waiting to receive\n");
		memset(buf1,'\0',1000);
        	rc = recvfrom(socketb, buf1, 1000, 0, &senderaddress, &slen);
        	if (rc == 0)
        		printf("Receive failed\n");
        	else if (rc == -1)
        		printf("recv() failed\n");
		cmd = strtok(buf1,",");
		printf("REceived cmd %s\n",cmd);

		getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
		printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
		printf("\nSocket Command: command recieved:%s",cmd);
		//Write some breaking function here..
		// As of now, Idea is to keep waiting, if u want to break.. set a variable somewhere.. notify some node, which will ping back in acknowledgement..
		// This will make me reach this point from recvfrom.. and i will do a break here..
		if (strcmp(cmd,GETATTR)==0)
		{
     		printf("\nSocket Command: in GETATTR");
			char *path;
			path = strtok(NULL,",");
			printf("User is trying to access something.. \n");
			//printf("What do I do, naan enna seyya.. \n");
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
			char *path = NULL;
			path = strtok(NULL, ",");
			printf("Path : %s\n",path);
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
		else if(strcmp(cmd,READ)==0)
		{
			printf("\nIn read\n");
			char *path = strtok(NULL,",");
			printf("Path : %s\n",path);
			char *size = strtok(NULL,",");
			printf("Size : %s\n",size);
			int sz;
			if (size == NULL)
				sz = 0;
			else
				sz = atoi(size);
			char *offset = strtok(NULL,",");
			printf("Offset : %s\n", offset);
			int off;
			if ( offset == NULL)
				off = 0;
			else
				off = atoi(offset);
			if (sendresponse_read(host,path,sz,off) == -1)
				continue;
		}
		else if(strcmp(cmd,WRITE)==0)
                {
                        printf("In write\n");
                        char *path = strtok(NULL,",");
                        printf("Path : %s\n",path);
                        char *size = strtok(NULL,",");
                        printf("Size : %s\n",size);
                        int sz;
                        if (size == NULL) sz = 0;
                        else sz = atoi(size);
                        char *offset = strtok(NULL,",");
                        printf("Offset : %s\n", offset);
                        int off;
                        if (offset == NULL) off = 0;
                        else off = atoi(offset);
			char *w_buf = strtok(NULL,",");
                        if (sendresponse_write(host,path,sz,off,w_buf) == -1)
                                continue;
                }
		else if(strcmp(cmd,MKDIR)==0)
		{
			printf("In MKDIR\n");
			char *path;
			path = strtok(NULL, ",");
			printf("mkdir found path path:%s\n",path);
			int imode=	atoi(strtok(NULL, ","));	
			printf("in mkdir path:%s host:%s \n",path,host);
			if (sendresponse_mkdir(host,path,imode) == -1)
				continue;
		}
		else if(strcmp(cmd,MKNOD)==0)
		{
			printf("In mknod\n");
			char *path;
			path = strtok(NULL,",");
			char *mode_c = strtok(NULL,",");
			printf("mode_c %s\n", mode_c);
			int mode = 0;
			if (mode_c != NULL) mode = atoi(mode_c);
			char *rdev_c = strtok(NULL,",");
			printf("rdev_c %s\n", rdev_c);
			int rdev = 0;
			if (rdev_c != NULL) rdev = atoi(rdev_c);
			if (sendresponse_mknod(host,path,mode,rdev) == -1)
				continue;
		}
		else if(strcmp(cmd,TRUNCATE)==0)
		{
			printf("In truncate\n");
			char *path;
			path = strtok(NULL,",");
			char *size_c = strtok(NULL,",");
			int size = 0;
			if (size_c != NULL) size = atoi(size_c);
			sendresponse_truncate(host,path,size);
		}
		else if(strcmp(cmd,UNLINK)==0)
		{
			printf("In unlink\n");
			char *path;
			path = strtok(NULL,",");
			sendresponse_unlink(host,path);
		}
		else if(strcmp(cmd,RMDIR)==0)
		{
			printf("In rmdir\n");
			char *path;
			path = strtok(NULL,",");
			sendresponse_rmdir(host,path);
		}
		
		else if(strcmp(cmd,STATFS)==0)
		{
			printf("In statfs\n");
			char *path;
			path = strtok(NULL,",");
			sendresponse_statfs(host,path);
		}
		else if(strcmp(cmd,CHMOD)==0)
		{
			printf("In CHMOD\n");
			char *path;
			path = strtok(NULL, ",");
			int imode=	atoi(strtok(NULL, ","));	
			printf("in chmod path:%s host:%s \n",path,host);
			if (sendresponse_chmod(host,path,imode) == -1)
				continue;
		}

		else if(strcmp(cmd,RENAME)==0)
		{
			printf("In RENAME\n");
			char *path;
			char *dpath;
			path = strtok(NULL, ",");
			dpath=strtok(NULL, ",");	
			printf("in rename path:%s dpath:%s host:%s \n",path,dpath,host);
			if (sendresponse_rename(host,path,dpath) == -1)
				continue;
		}
		else
		{
			printf("Unknown command %s\n",cmd);
		}
		cmd = NULL;
	}
	shutdown(socketb,2);
	return 0;
}

/****************************************************
*****************************************************
Receive for Namenode
*****************************************************/

int receivecommand_namenode()
{
	printf("Receive namenode\n");
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
        receiveaddress.sin_port = htons(COMMAND_PORT);
        if (bind(socketb,&receiveaddress, sizeof(receiveaddress)) > 0)
	{
		printf("Bind error\n");
		exit(1);
	}

        // receive message
  char buf1[1000];
  char buf2[1000];
	memset (buf1,'\0',1000);
	int slen = sizeof(senderaddress);
	int rc = 0;
	char host[100];
	char serv[100];
	char *cmd;
	while (1)
	{
		printf("Waiting to receive - Namenode\n");
		memset(buf1,'\0',1000);
	
				strcpy(buf2,buf1);        	
        	rc = recvfrom(socketb, buf1, 1000, 0, &senderaddress, &slen);

        	if (rc == 0)
        		printf("Receive failed\n");
        	else if (rc == -1)
        		printf("recv() failed\n");

		getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
		printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
			memset(buf2,'\0',1000);
			strcpy(buf2,"");
			strcat(buf2,host);
			strcat(buf2,",");
			strcat(buf2,buf1);
			printf("Buf 1 %s\n",buf1);
			printf("Printing buffer storage %s",buf2);
		cmd = strtok(buf1,",");
		printf("REceived cmd %s\n",cmd);
		
		printf("\nSocket Command: command recieved:%s",cmd);
		//Write some breaking function here..
		// As of now, Idea is to keep waiting, if u want to break.. set a variable somewhere.. notify some node, which will ping back in acknowledgement..
		// This will make me reach this point from recvfrom.. and i will do a break here..
		if (strcmp(cmd,GETATTR)==0)
		{
     		printf("\nSocket Command: in GETATTR");
			char *path;
			path = strtok(NULL,",");
			//Return a datanode, where file can be created
			dpath = getdatanode(path,host);

			
			printf("User is trying to access something..%s \n",dpath);
			      if (sendcommand(dpath, buf2, GETATTR) == -1)
        {
                printf("Send command to datanode failed\n");
                return -1;
        }
			//printf("What do I do, naan enna seyya.. \n");
			//May be intimate some process.
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
			dpath = getdatanode(path,host);
			printf("User is trying to access something..%s \n",dpath);
	      if (sendcommand(dpath, buf2, ACCESS) == -1)
        {
                printf("Send command to datanode failed\n");
								continue;
        }
				
		}
		else if(strcmp(cmd,READDIR)==0)
		{
			printf("In readdir\n");
			char *path = NULL;
			path = strtok(NULL, ",");
			printf("Path : %s\n",path);
			dpath = getdatanode(path,host);
			printf("Doing a Readdir.. \n");
	      if (sendcommand(dpath, buf2, READDIR) == -1)
        {
                printf("Send command to datanode failed\n");
								continue;
        }
		}
		else if(strcmp(cmd,OPEN)==0)
		{
			printf("In open\n");
			char *path = strtok(NULL,",");
			printf("Path : %s\n",path);
			dpath = getdatanode(path,host);
			printf("Open file.. \n");
	      if (sendcommand(dpath, buf2, OPEN) == -1)
        {
                printf("Send command to datanode failed\n");
								continue;
        }

		}
		else if(strcmp(cmd,READ)==0)
		{
			printf("\nIn read\n");
			char *path = strtok(NULL,",");
			dpath = getdatanode(path,host);
			printf("User is trying Read.. \n");
	      if (sendcommand(dpath, buf2, READ) == -1)
        {
                printf("Send command to datanode failed\n");
								continue;
        }
		}
		else if(strcmp(cmd,WRITE)==0)
    {
      printf("In write\n");
      char *path = strtok(NULL,",");
			dpath = getdatanode(path,host);
			printf("Write!!!!!!!!!!!!.. \n");
      if (sendcommand(dpath, buf2, WRITE) == -1)
      {
          printf("Send command to datanode failed\n");
					continue;
      }
    }
		else if(strcmp(cmd,MKDIR)==0)
		{
			printf("In MKDIR\n");
			char *path;
			path = strtok(NULL, ",");
			printf("mkdir found path path:%s\n",path);
			dpath = getdatanode(path,host);
			printf("I am making a dir.. dpath:%s\n",dpath);
	      if (sendcommand(dpath, buf2, MKDIR) == -1)
        {
                printf("Send command to datanode failed\n");
								continue;
        }
		}
		else if(strcmp(cmd,MKNOD)==0)
		{
			printf("In mknod\n");
			char *path;
			path = strtok(NULL,",");
			dpath = getdatanode(path,host);
			printf("Making Node.... \n");
	      if (sendcommand(dpath, buf2, MKNOD) == -1)
        {
                printf("Send command to datanode failed\n");
								continue;
        }

		}
		else if(strcmp(cmd,TRUNCATE)==0)
		{
			printf("In truncate\n");
			char *path;
			path = strtok(NULL,",");
			dpath = getdatanode(path,host);
			printf("Truncate .... \n");
	      if (sendcommand(dpath, buf2, TRUNCATE) == -1)
        {
                printf("Send command to datanode failed\n");
								continue;
        }
		}
		else if(strcmp(cmd,UNLINK)==0)
		{
			printf("In unlink\n");
			char *path;
			path = strtok(NULL,",");
			dpath = getdatanode(path,host);
			printf("Yeah i am unlinking.. \n");
	      if (sendcommand(dpath, buf2, UNLINK) == -1)
        {
                printf("Send command to datanode failed\n");
								continue;
        }

		}
		else if(strcmp(cmd,RMDIR)==0)
		{
			printf("In rmdir\n");
			char *path;
			path = strtok(NULL,",");
			dpath = getdatanode(path,host);
			printf("Remove Directory.. \n");
	      if (sendcommand(dpath, buf2, RMDIR) == -1)
        {
                printf("Send command to datanode failed\n");
								continue;
        }
		}
		
		else if(strcmp(cmd,STATFS)==0)
		{
			printf("In statfs\n");
			char *path;
			path = strtok(NULL,",");
			sendresponse_statfs(host,path);
		}
		else if(strcmp(cmd,CHMOD)==0)
		{
			printf("In CHMOD\n");
			char *path;
			path = strtok(NULL, ",");
			dpath = getdatanode(path,host);
			printf("Changing permission.. \n");
	      if (sendcommand(dpath, buf2, CHMOD) == -1)
        {
                printf("Send command to datanode failed\n");
								continue;
        }

		}

		else if(strcmp(cmd,RENAME)==0)
		{
			printf("In RENAME\n");
			char *path;
			char *dpath;
			path = strtok(NULL, ",");
			dpath = getdatanode(path,host);
			printf("Renaming a file.. \n");
	      if (sendcommand(dpath, buf2, RENAME) == -1)
        {
                printf("Send command to datanode failed\n");
								continue;
        }
		}
		else
		{
			printf("Unknown command %s\n",cmd);
		}
		cmd = NULL;
	}
	shutdown(socketb,2);
	return 0;
}

/***************************************************
****************************************************
This would run on the datanode

****************************************************/
int receivecommand_datanode()
{
	printf("Receive datanode\n");
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
	char *tmphost;
	char namehost[100];	
	char serv[100];
	char *cmd;
	while (1)
	{
		printf("Waiting to receive\n");
		memset(buf1,'\0',1000);
        	rc = recvfrom(socketb, buf1, 1000, 0, &senderaddress, &slen);
        	if (rc == 0)
        		printf("Receive failed\n");
        	else if (rc == -1)
        		printf("recv() failed\n");
		tmphost = strtok(buf1,",");
		tmphost = strtok(NULL,",");
		strcpy(host,tmphost);
		cmd = strtok(NULL,",");
		printf("REceived cmd %s host:%s temphost:%s\n ",cmd,host,tmphost);

		getnameinfo(&senderaddress, slen, namehost, sizeof(namehost), serv, sizeof(serv), 0);
		printf("Command received from %s, host %s, client:%s\n",inet_ntoa(senderaddress.sin_addr),namehost,host);
		printf("\nSocket Command: command recieved:%s",cmd);
		//Write some breaking function here..
		// As of now, Idea is to keep waiting, if u want to break.. set a variable somewhere.. notify some node, which will ping back in acknowledgement..
		// This will make me reach this point from recvfrom.. and i will do a break here..
		if (strcmp(cmd,GETATTR)==0)
		{
     		printf("\nSocket Command: in GETATTR");
			char *path;
			path = strtok(NULL,",");
			printf("User is trying to access something.. \n");
			//printf("What do I do, naan enna seyya.. \n");
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
			char *path = NULL;
			path = strtok(NULL, ",");
			printf("Path : %s\n",path);
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
		else if(strcmp(cmd,READ)==0)
		{
			printf("\nIn read\n");
			char *path = strtok(NULL,",");
			printf("Path : %s\n",path);
			char *size = strtok(NULL,",");
			printf("Size : %s\n",size);
			int sz;
			if (size == NULL)
				sz = 0;
			else
				sz = atoi(size);
			char *offset = strtok(NULL,",");
			printf("Offset : %s\n", offset);
			int off;
			if ( offset == NULL)
				off = 0;
			else
				off = atoi(offset);
			if (sendresponse_read(host,path,sz,off) == -1)
				continue;
		}
		else if(strcmp(cmd,WRITE)==0)
                {
                        printf("In write\n");
                        char *path = strtok(NULL,",");
                        printf("Path : %s\n",path);
                        char *size = strtok(NULL,",");
                        printf("Size : %s\n",size);
                        int sz;
                        if (size == NULL) sz = 0;
                        else sz = atoi(size);
                        char *offset = strtok(NULL,",");
                        printf("Offset : %s\n", offset);
                        int off;
                        if (offset == NULL) off = 0;
                        else off = atoi(offset);
			char *w_buf = strtok(NULL,",");
                        if (sendresponse_write(host,path,sz,off,w_buf) == -1)
                                continue;
                }
		else if(strcmp(cmd,MKDIR)==0)
		{
			printf("In MKDIR\n");
			char *path;
			path = strtok(NULL, ",");
			printf("mkdir found path path:%s\n",path);
			int imode=	atoi(strtok(NULL, ","));	
			printf("in mkdir path:%s host:%s \n",path,host);
			if (sendresponse_mkdir(host,path,imode) == -1)
				continue;
		}
		else if(strcmp(cmd,MKNOD)==0)
		{
			printf("In mknod\n");
			char *path;
			path = strtok(NULL,",");
			char *mode_c = strtok(NULL,",");
			printf("mode_c %s\n", mode_c);
			int mode = 0;
			if (mode_c != NULL) mode = atoi(mode_c);
			char *rdev_c = strtok(NULL,",");
			printf("rdev_c %s\n", rdev_c);
			int rdev = 0;
			if (rdev_c != NULL) rdev = atoi(rdev_c);
			if (sendresponse_mknod(host,path,mode,rdev) == -1)
				continue;
		}
		else if(strcmp(cmd,TRUNCATE)==0)
		{
			printf("In truncate\n");
			char *path;
			path = strtok(NULL,",");
			char *size_c = strtok(NULL,",");
			int size = 0;
			if (size_c != NULL) size = atoi(size_c);
			sendresponse_truncate(host,path,size);
		}
		else if(strcmp(cmd,UNLINK)==0)
		{
			printf("In unlink\n");
			char *path;
			path = strtok(NULL,",");
			sendresponse_unlink(host,path);
		}
		else if(strcmp(cmd,RMDIR)==0)
		{
			printf("In rmdir\n");
			char *path;
			path = strtok(NULL,",");
			sendresponse_rmdir(host,path);
		}
		
		else if(strcmp(cmd,STATFS)==0)
		{
			printf("In statfs\n");
			char *path;
			path = strtok(NULL,",");
			sendresponse_statfs(host,path);
		}
		else if(strcmp(cmd,CHMOD)==0)
		{
			printf("In CHMOD\n");
			char *path;
			path = strtok(NULL, ",");
			int imode=	atoi(strtok(NULL, ","));	
			printf("in chmod path:%s host:%s \n",path,host);
			if (sendresponse_chmod(host,path,imode) == -1)
				continue;
		}

		else if(strcmp(cmd,RENAME)==0)
		{
			printf("In RENAME\n");
			char *path;
			char *dpath;
			path = strtok(NULL, ",");
			dpath=strtok(NULL, ",");	
			printf("in rename path:%s dpath:%s host:%s \n",path,dpath,host);
			if (sendresponse_rename(host,path,dpath) == -1)
				continue;
		}
		else
		{
			printf("Unknown command %s\n",cmd);
		}
		cmd = NULL;
	}
	shutdown(socketb,2);
	return 0;
}





int sendresponse_chmod(char *node, char *path,int imode)
{
	printf("Send response chmod %s, imode: %d\n", path,imode);

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
		char msg[30];		
//		imode=0777;
		int res = chmod(path, imode);
//		printf("Send response chmod %s res:%d\n", path,res);


//	printf("Error # : %d\n", errno);
	char buffer[30];
	memset(msg,'\0',30);
	snprintf(buffer, 10,"%d",errno);

		if (res == -1)
			msg[0]='Y';
		else
			msg[0]='N';		
	strcat (msg,",");
	strcat (msg,buffer);



		printf("Send response chmod %s msg:%s\n", path,msg);
		int ret = sendto(socketa, &msg, strlen(msg), 0, &sendaddress, slen);
		
        if (ret == 0)
                printf("Send failed\n");
        else if (ret == -1)
                printf("send() failed\n");
		else
	        printf("Sending response done\n");
	
        close(socketa);
        return 0;
}


int sendresponse_rename(char *node, char *path,char *dpath)
{
	printf("Send response rename %s, imode: %s\n", path,dpath);

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
		char msg[30];		
//		imode=0777;
		int res = rename(path, dpath);
//		printf("Send response rename %s res:%d\n", path,res);


//	printf("Error # : %d\n", errno);
	char buffer[30];
	memset(msg,'\0',30);
	snprintf(buffer, 10,"%d",errno);

		if (res == -1)
			msg[0]='Y';
		else
			msg[0]='N';		
	strcat (msg,",");
	strcat (msg,buffer);



		printf("Send response rename %s msg:%s\n", path,msg);
		int ret = sendto(socketa, &msg, strlen(msg), 0, &sendaddress, slen);
		
        if (ret == 0)
                printf("Send failed\n");
        else if (ret == -1)
                printf("send() failed\n");
		else
	        printf("Sending response done\n");
	
        close(socketa);
        return 0;
}



int sendresponse_statfs(char *node, char *path)
{
        printf("Send response statfs %s\n", path);

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

	char statfsbuf[1000];
	memset(statfsbuf,'\0',1000);
	strcpy(statfsbuf,"");

	char buffer[30];
	memset(buffer,'\0',30);
	struct statvfs st;
	int res = statvfs(path, &st);

	snprintf(buffer, 10,"%d",res);
	strcat (statfsbuf,buffer);
	strcat (statfsbuf,",");

	printf("Error # : %d\n", errno);
	memset(buffer,'\0',30);
	snprintf(buffer, 10,"%d",errno);
	strcat (statfsbuf,buffer);
	strcat (statfsbuf,",");
	if(res != -1)
	{
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",st.f_bsize);
		strcat (statfsbuf,buffer);
		strcat (statfsbuf,",");
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",st.f_frsize);
		strcat (statfsbuf,buffer);
		strcat (statfsbuf,",");
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",st.f_blocks);
		strcat (statfsbuf,buffer);
		strcat (statfsbuf,",");
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",st.f_bfree);
		strcat (statfsbuf,buffer);
		strcat (statfsbuf,",");
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",st.f_bavail);
		strcat (statfsbuf,buffer);
		strcat (statfsbuf,",");
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",st.f_files);
		strcat (statfsbuf,buffer);
		strcat (statfsbuf,",");
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",st.f_ffree);
		strcat (statfsbuf,buffer);
		strcat (statfsbuf,",");
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",st.f_favail);
		strcat (statfsbuf,buffer);
		strcat (statfsbuf,",");
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",st.f_fsid);
		strcat (statfsbuf,buffer);
		strcat (statfsbuf,",");
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",st.f_flag);
		strcat (statfsbuf,buffer);
		strcat (statfsbuf,",");
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",st.f_namemax);
		strcat (statfsbuf,buffer);
		strcat (statfsbuf,",");
	}

	memset(buffer,'\0',30);
	snprintf(buffer, 10,"%d",errno);
	strcat (statfsbuf,buffer);
	strcat (statfsbuf,",");
	
        int ret = sendto(socketa, &statfsbuf, strlen(statfsbuf), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed with 0\n");
        else if (ret == -1) printf("send() failed with 1\n");
	else printf("Sending response done\n");
        close(socketa);
        return 0;
}


int sendresponse_rmdir(char *node, char *path)
{
        printf("Send response rmdir %s\n", path);

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

	int res = rmdir(path);
	
	printf("Error # : %d\n", errno);
	char rmdirbuf[100];
	memset(rmdirbuf,'\0',100);
	strcpy(rmdirbuf,"");

	char buffer[30];
	memset(buffer,'\0',30);
	snprintf(buffer, 10,"%d",res);
	strcat (rmdirbuf,buffer);
	strcat (rmdirbuf,",");

	memset(buffer,'\0',30);
	snprintf(buffer, 10,"%d",errno);
	strcat (rmdirbuf,buffer);
	strcat (rmdirbuf,",");
	
        int ret = sendto(socketa, &rmdirbuf, strlen(rmdirbuf), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed with 0\n");
        else if (ret == -1) printf("send() failed with 1\n");
	else printf("Sending response done\n");
        close(socketa);
        return 0;
}

int sendresponse_unlink(char *node, char *path)
{
        printf("Send response unlink %s\n", path);

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

	int res = unlink(path);
	
	printf("Error # : %d\n", errno);
	char unlinkbuf[100];
	memset(unlinkbuf,'\0',100);
	strcpy(unlinkbuf,"");

	char buffer[30];
	memset(buffer,'\0',30);
	snprintf(buffer, 10,"%d",res);
	strcat (unlinkbuf,buffer);
	strcat (unlinkbuf,",");

	memset(buffer,'\0',30);
	snprintf(buffer, 10,"%d",errno);
	strcat (unlinkbuf,buffer);
	strcat (unlinkbuf,",");
	
        int ret = sendto(socketa, &unlinkbuf, strlen(unlinkbuf), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed with 0\n");
        else if (ret == -1) printf("send() failed with 1\n");
	else printf("Sending response done\n");
        close(socketa);
        return 0;
}

int sendresponse_truncate(char *node, char *path, int size)
{
        printf("Send response truncate %s %d\n", path, size);

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

	int res = truncate(path,size);
	
	printf("Error # : %d\n", errno);
	char truncatebuf[100];
	memset(truncatebuf,'\0',100);
	strcpy(truncatebuf,"");

	char buffer[30];
	memset(buffer,'\0',30);
	snprintf(buffer, 10,"%d",res);
	strcat (truncatebuf,buffer);
	strcat (truncatebuf,",");

	memset(buffer,'\0',30);
	snprintf(buffer, 10,"%d",errno);
	strcat (truncatebuf,buffer);
	strcat (truncatebuf,",");
	
        int ret = sendto(socketa, &truncatebuf, strlen(truncatebuf), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed with 0\n");
        else if (ret == -1) printf("send() failed with 1\n");
	else printf("Sending response done\n");
        close(socketa);
        return 0;
}
int sendresponse_mknod(char *node, char *path, int mode, int rdev)
{
        printf("Send response mknod %s %d %d\n", path, mode, rdev);

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

	int res;
	//if (S_ISREG(mode))
	//{
		printf("hi if\n");
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0) res = close(res);
/*	}
	else if (S_ISFIFO(mode))
	{
		printf("hi else if\n");
		res = mkfifo(path, mode);
	}
	else
	{
		printf("else\n");
		res = mknod(path, mode, rdev);
	}
	*/
	printf("Error # : %d\n", errno);
	char mknodbuf[100];
	memset(mknodbuf,'\0',100);
	strcpy(mknodbuf,"");

	char buffer[30];
	memset(buffer,'\0',30);
	snprintf(buffer, 10,"%d",res);
	strcat (mknodbuf,buffer);
	strcat (mknodbuf,",");

	memset(buffer,'\0',30);
	snprintf(buffer, 10,"%d",errno);
	strcat (mknodbuf,buffer);
	strcat (mknodbuf,",");
	
        int ret = sendto(socketa, &mknodbuf, strlen(mknodbuf), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed with 0\n");
        else if (ret == -1) printf("send() failed with 1\n");
	else printf("Sending response done\n");
        close(socketa);
        return 0;
}

int sendresponse_write(char *node, char *path, int size, int offset, char *buf)
{
        printf("Send response write %s %d %d %s\n", path, size, offset, buf);
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

	char writebuf[100];
	char buffer[30];
	memset(buffer,'\0',30);
	memset(writebuf,'\0',100);
	strcpy(writebuf,"");
        int ret,res;
        int fd = open(path, O_WRONLY);
        if (fd == -1)
	{
		strcat(writebuf,"-1");
		strcat(writebuf,",");

		snprintf(buffer, 10,"%d",errno);
		strcat (writebuf,buffer);
	}
	else
	{
        	res = pwrite(fd, buf, size, offset);
        	if (res == -1)
                	res = -errno;

		snprintf(buffer, 10,"%d",res);
		strcat(writebuf,buffer);
		strcat(writebuf,",");

		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",errno);
		strcat (writebuf,buffer);
	
        	close(fd);
	}

        ret = sendto(socketa, &writebuf, strlen(writebuf), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed\n");
        else if (ret == -1) printf("send() failed\n");
 	else printf("Sending response done\n");

        close(socketa);
        return 0;
}


int sendresponse_read(char *node, char *path, int size, int offset)
{
        printf("Send response read %s %d %d\n", path, size, offset);
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

	char readbuf[10000];
	memset(readbuf,'\0',10000);
	strcpy(readbuf,"");
	char buffer[30];
	memset(buffer,'\0',30);
        int res;
	int fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		strcat(readbuf,"-1");
		strcat(readbuf,",");

		snprintf(buffer, 10,"%d",errno);
		strcat (readbuf,buffer);
	}
	else
	{
		char buf[4096];
		memset(buf,'\0',4096);
        	res = pread(fd, buf, size, offset);
        	close(fd);
		printf("buf : %s\n",buf);

		snprintf(buffer, 10,"%d",res);
		strcat(readbuf,buffer);
		strcat(readbuf,",");

		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",errno);
		strcat(readbuf,buffer);
		strcat(readbuf,",");

		strcat(readbuf,buf);
	}
        int ret = sendto(socketa, &readbuf, strlen(readbuf), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed\n");
        else if (ret == -1) printf("send() failed\n");
	else printf("Sending response done\n");
	printf("Buffer %s\n",readbuf);
        close(socketa);
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

	char openbuf[100];
	memset(openbuf,'\0',100);
	strcpy(openbuf,"");
	char buffer[30];
	memset(buffer,'\0',30);
        int res = open(path, flags);
        if (res == -1)
        {
		strcat(openbuf,"-1");
		strcat(openbuf,",");

		snprintf(buffer, 10,"%d",errno);
		strcat (openbuf,buffer);
        }
        else
	{
		close(res);
		snprintf(buffer, 10,"%d",res);
		strcat(openbuf,buffer);
		strcat(openbuf,",");

		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",errno);
		strcat (openbuf,buffer);
	}
        int ret = sendto(socketa, &openbuf, strlen(openbuf), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed\n");
        else if (ret == -1) printf("send() failed\n");
	else printf("Sending response done\n");

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

	char accessbuf[100];
	memset(accessbuf,'\0',100);
	strcpy(accessbuf,"");
	char buffer[30];
	memset(buffer,'\0',30);

	snprintf(buffer,10,"%d",res);
	strcat(accessbuf,buffer);
	strcat(accessbuf,",");
	
	memset(buffer,'\0',30);
	snprintf(buffer,10,"%d",errno);
	strcat(accessbuf,buffer);

	ret = sendto(socketa, &accessbuf, strlen(accessbuf), 0, &sendaddress, slen);
	if (ret == 0) printf("Send failed\n");
        else if (ret == -1) printf("send() failed\n");
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
	printf("After hostname\n");
        sendaddress.sin_family = AF_INET;
        sendaddress.sin_port = htons(RESPONSE_PORT);
        if (inet_aton(ip, &sendaddress.sin_addr)==0) {
                printf(stderr, "inet_aton() failed\n");
                exit(1);
        }

	int count = 0;
	DIR *dp = NULL;
	struct dirent *de = NULL;
	
	char listfiles[10000];
	memset(listfiles,'\0',10000);
	strcpy(listfiles,"");

	char buffer[30];
	dp = opendir(path);
	if (dp == NULL)
	{
		printf("DP == Null\n");
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",errno);
		strcat(listfiles, buffer);
		strcat(listfiles, ",");
		
	}
	else
	{
		memset(buffer,'\0',30);
		snprintf(buffer, 10,"%d",errno);
		strcat(listfiles, buffer);
		strcat(listfiles, ",");
		while ((de = readdir(dp)) != NULL)
		{
			printf("d_name %s\n",de->d_name);
			printf("d_type %d\n",de->d_type);
			printf("d_ino %d\n",de->d_ino);
			strcat(listfiles,de->d_name);
			strcat(listfiles,",");
			memset(buffer,'\0',30);
			snprintf(buffer, 10,"%d",de->d_type);
			strcat(listfiles, buffer);
			strcat(listfiles, ",");

			memset(buffer,'\0',30);
			snprintf(buffer, 10,"%d",de->d_ino);
			strcat(listfiles, buffer);
			strcat(listfiles,",");
		}
		closedir(dp);
	}
	printf("Am I here\n");
	int ret = sendto(socketa, listfiles, strlen(listfiles), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed\n");
        else if (ret == -1) printf("send() failed\n");
        else printf("Sending response done\n");
	
        close(socketa);
        return 0;
}

int sendresponse_mkdir(char *node, char *path,int imode)
{
	printf("Send response mkdir %s, imode: %d\n", path,imode);

	int socketa = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        struct sockaddr_in sendaddress;
        int slen = sizeof(sendaddress);
		printf("Send response mkdir %s llen:%d\n", path,slen);
        char ip[100];
        if (hostname_to_ip(node, ip) == -1)
                return -1;
        sendaddress.sin_family = AF_INET;
        sendaddress.sin_port = htons(RESPONSE_PORT);
        if (inet_aton(ip, &sendaddress.sin_addr)==0) {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }
		char msg[30];		
//		imode=0777;
		int res = mkdir(path, imode);
		printf("Send response mkdir %s res:%d\n", path,res);


	printf("Error # : %d\n", errno);
	char buffer[30];
	memset(msg,'\0',30);
	snprintf(buffer, 10,"%d",errno);

		if (res == -1)
			msg[0]='Y';
		else
			msg[0]='N';		
	strcat (msg,",");
	strcat (msg,buffer);



		printf("Send response mkdir %s msg:%s\n", path,msg);
		int ret = sendto(socketa, &msg, strlen(msg), 0, &sendaddress, slen);
		
        if (ret == 0)
                printf("Send failed\n");
        else if (ret == -1)
                printf("send() failed\n");
		else
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
	int res = stat(path,&st);
	printf("sendresp get attr stat res:%d, path:%s",res, path);
	char gattr_b[1000];
        memset(gattr_b,'\0',1000);
	strcpy(gattr_b,"");
	if(res == -1)
	{
		strcat(gattr_b, "-1");//res
		strcat(gattr_b, ",");
		char buffer[30];
        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",errno);
		strcat(gattr_b,buffer);
	}
	else
	{
		printf("sendresp get atrr: else part\n");
		char buffer[30];
        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",res);
		strcat(gattr_b, buffer);//res
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",errno);
		strcat(gattr_b, buffer);//errno
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",st.st_mode);
		strcat(gattr_b,buffer);//mode
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",st.st_ino);
		strcat(gattr_b,buffer);//ino
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",st.st_dev);
		strcat(gattr_b,buffer);//dev
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",st.st_rdev);
		strcat(gattr_b,buffer);//rdev
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",st.st_nlink);
		strcat(gattr_b,buffer);//nlink
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",st.st_uid);
		strcat(gattr_b,buffer);//uid
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",st.st_gid);
		strcat(gattr_b,buffer);//gid
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",st.st_size);
		strcat(gattr_b,buffer);//size
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",st.st_atime);
		strcat(gattr_b,buffer);//atime
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",st.st_mtime);
		strcat(gattr_b,buffer);//mtime
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",st.st_ctime);
		strcat(gattr_b,buffer);//ctime
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",st.st_blksize);
		strcat(gattr_b,buffer);//blksize
		strcat(gattr_b, ",");

        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",st.st_blocks);
		strcat(gattr_b,buffer);//blocks
		printf("End of else\n");
	}
	// I should load contents in st before I send..
        int ret = sendto(socketa, &gattr_b, strlen(gattr_b), 0, &sendaddress, slen);
        if (ret == 0) printf("Send failed\n");
        else if (ret == -1) printf("send() failed\n");
        else printf("Sending response done\n");
        //shutdown(socketa,2);
	close(socketa);
        return 0;
}

int receiveresponse_client()
{
	printf("Receive command client\n");
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
		memset(getattr_buf,'\0',1000);
        	printf("Waiting to receive Getattr\n");
        	rc = recvfrom(socketb, &getattr_buf, 1000, 0, &senderaddress, &slen);
        	if (rc == 0) printf("Receive failed\n");
        	else if (rc == -1) printf("recv() failed\n");
		printf("Getattrbuf : %s\n", getattr_buf);
        	getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
        	printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, READDIR)==0)
	{
		printf("Readdir client receive\n");
		memset(readdir_buf,'\0',10000);
		rc = recvfrom(socketb, &readdir_buf, 10000, 0, &senderaddress, &slen);
                if (rc == 0) printf("Receive failed\n");
                else if (rc == -1) printf("recv() failed\n");
		printf("Count : %s\n", readdir_buf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, ACCESS) == 0)
	{
		rc = recvfrom(socketb, &access_retbuf, 100, 0, &senderaddress, &slen);
		if (rc == 0) printf("Receive failed\n");
                else if (rc == -1) printf("recv() failed\n");
                printf("Acess Return : %s\n", access_retbuf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, OPEN) == 0)
	{
		rc = recvfrom(socketb, &open_retbuf, 100, 0, &senderaddress, &slen);
                if (rc == 0) printf("Receive failed\n");
                else if (rc == -1) printf("recv() failed\n");
                printf("Open Return : %s\n",open_retbuf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, READ) == 0)
	{
		rc = recvfrom(socketb, &read_retbuf, 10000, 0, &senderaddress, &slen);
                if (rc == 0) printf("Receive failed\n");
                else if (rc == -1) printf("recv() failed\n");
                printf("Read Return : %s\n",read_retbuf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, WRITE) == 0)
	{
		rc = recvfrom(socketb, &write_retbuf, 100, 0, &senderaddress, &slen);
                if (rc == 0) printf("Receive failed\n");
                else if (rc == -1) printf("recv() failed\n");
                printf("Write Return : %s\n",write_retbuf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, MKNOD) == 0)
	{
		rc = recvfrom(socketb, &mknod_retbuf, 100, 0, &senderaddress, &slen);
                if (rc == 0) printf("Receive failed\n");
                else if (rc == -1) printf("recv() failed\n");
                printf("Mknod Return : %s\n",mknod_retbuf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, TRUNCATE) == 0)
	{
		memset(truncate_retbuf,'\0',100);
		rc = recvfrom(socketb, &truncate_retbuf, 100, 0, &senderaddress, &slen);
                if (rc == 0) printf("Receive failed\n");
                else if (rc == -1) printf("recv() failed\n");
                printf("Truncate Return : %s\n",truncate_retbuf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, UNLINK) == 0)
	{
		memset(unlink_retbuf,'\0',100);
		rc = recvfrom(socketb, &unlink_retbuf, 100, 0, &senderaddress, &slen);
                if (rc == 0) printf("Receive failed\n");
                else if (rc == -1) printf("recv() failed\n");
                printf("Unlink Return : %s\n",unlink_retbuf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, RMDIR) == 0)
	{
		memset(rmdir_retbuf,'\0',100);
		rc = recvfrom(socketb, &rmdir_retbuf, 100, 0, &senderaddress, &slen);
                if (rc == 0) printf("Receive failed\n");
                else if (rc == -1) printf("recv() failed\n");
                printf("Rmdir Return : %s\n",rmdir_retbuf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, STATFS) == 0)
	{
		memset(statvfs_retbuf,'\0',1000);
		rc = recvfrom(socketb, &statvfs_retbuf, 1000, 0, &senderaddress, &slen);
                if (rc == 0) printf("Receive failed\n");
                else if (rc == -1) printf("recv() failed\n");
                printf("Statfs Return : %s\n",statvfs_retbuf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, MKDIR)==0)
	{
		printf("receiverresponse_client before in MKDIR \n");
		memset(mkdir_buf,'\0',10000);
		rc = recvfrom(socketb, &mkdir_buf, 10000, 0, &senderaddress, &slen);
                if (rc == 0)
                        printf("Receive failed\n");
                else if (rc == -1)
                        printf("recv() failed\n");
		printf("Count : %s\n", mkdir_buf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, CHMOD)==0)
	{
		printf("receiverresponse_client before in CHMOD \n");
		memset(chmod_buf,'\0',10);
		rc = recvfrom(socketb, &chmod_buf, 10, 0, &senderaddress, &slen);
                if (rc == 0)
                        printf("Receive failed\n");
                else if (rc == -1)
                        printf("recv() failed\n");
								printf("Count : %s\n", chmod_buf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	
	else if (strcmp(COMMAND_NAME, RENAME)==0)
	{
		printf("receiverresponse_client before in RENAME \n");
		memset(rename_buf,'\0',10);
		rc = recvfrom(socketb, &rename_buf, 10, 0, &senderaddress, &slen);
                if (rc == 0)
                        printf("Receive failed\n");
                else if (rc == -1)
                        printf("recv() failed\n");
								printf("Count : %s\n", rename_buf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}

        close(socketb);
	return 0;
}
