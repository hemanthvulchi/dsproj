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
#include <fcntl.h>
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
			printf("In read\n");
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
			char *w_buf = strtok(NULL,",");
                        if (sendresponse_write(host,path,sz,off,w_buf) == -1)
                                continue;
                }
		else if(strcmp(cmd,MKDIR==0))
		{
			printf("In MKDIR\n");
			char *path;
			path = strtok(NULL, ",");
			int imode=	atoi(strtok(NULL, ","));			
			if (sendresponse_mkdir(host,path,imode) == -1)
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

        int ret,res;
        int fd = open(path, O_WRONLY);
        if (fd == -1)
	{
                res = -errno;
	}
	else
	{
        	res = pwrite(fd, buf, size, offset);
        	if (res == -1)
                	res = -errno;
        	close(fd);
	}

        ret = sendto(socketa, &res, sizeof(res), 0, &sendaddress, slen);
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

        int ret,res;
	int fd = open(path, O_RDONLY);
	if (fd == -1)
                return -errno;

	char buf[4096];
        res = pread(fd, buf, size, offset);
        if (res == -1)
                res = -errno;
        close(fd);
	printf("buf : %s\n",buf);

        res = -errno;
        ret = sendto(socketa, &buf, strlen(buf), 0, &sendaddress, slen);
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
        int res = open(path, flags);
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
	
	dp = opendir(path);
	//dp = opendir("/tmp/shyam-fuse");
	printf("I am here\n");
	if (dp == NULL)
	{
		printf("DP == Null\n");
		return -errno;
	}
	printf("I am here\n");

	
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
	closedir(dp);
	printf("Am I here\n");
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


int sendresponse_mkdir(char *node, char *path,int imode)
{
	printf("Send response mkdir %s\n", path);
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
		char msg;		
		int res = mkdir(path, imode);
		if (res == -1)
			msg='N';
		else
			msg='Y';		
	
		int ret = sendto(socketa, msg, strlen(msg), 0, &sendaddress, slen);

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
	int res = stat(path,&st);
	printf("sendresp get attr stat res:%d, path:%s",res, path);
	char gattr_b[1000];
        memset(gattr_b,'\0',1000);
	strcpy(gattr_b,"");
	if(res == -1)
	{
		strcat(gattr_b, "-1");//res
	}
	else
	{
		printf("sendresp get atrr: else part\n");
		//printf("sendresp get atrr: else part:%s\n",st);
		char buffer[30];
        	memset(buffer,'\0',30);
		snprintf(buffer,10,"%d",res);
		strcat(gattr_b, buffer);//res
		strcat(gattr_b, ",");
		printf("After res\n");

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
		memset(getattr_buf,'\0',1000);
        	printf("Waiting to receive Getattr\n");
        	rc = recvfrom(socketb, &getattr_buf, 1000, 0, &senderaddress, &slen);
        	if (rc == 0)
        		printf("Receive failed\n");
        	else if (rc == -1)
        		printf("recv() failed\n");
        	getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
        	printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
                //printf("Received packet from %s : %d bytes\nData: %s\n\n",inet_ntoa(senderaddress.sin_addr), ntohs(senderaddress.sin_port), buf1);
	}
	else if (strcmp(COMMAND_NAME, READDIR)==0)
	{
		printf("Readdir client receive\n");
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
	else if (strcmp(COMMAND_NAME, READ) == 0)
	{
		rc = recvfrom(socketb, &read_buf, 4096, 0, &senderaddress, &slen); //4096 because I am hardcoding read_buf to 4096 in global.h
                if (rc == 0)
                        printf("Receive failed\n");
                else if (rc == -1)
                        printf("recv() failed\n");
                printf("Read Return : %s\n",read_buf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, WRITE) == 0)
	{
		rc = recvfrom(socketb, &write_return, sizeof(int), 0, &senderaddress, &slen);
                if (rc == 0)
                        printf("Receive failed\n");
                else if (rc == -1)
                        printf("recv() failed\n");
                printf("Write Return : %d\n",write_return);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	else if (strcmp(COMMAND_NAME, MKDIR)==0)
	{
		memset(readdir_buf,'\0',10000);
		rc = recvfrom(socketb, &mkdir_buf, 10000, 0, &senderaddress, &slen);
                if (rc == 0)
                        printf("Receive failed\n");
                else if (rc == -1)
                        printf("recv() failed\n");
								printf("Count : %s\n", mkdir_buf);
                getnameinfo(&senderaddress, slen, host, sizeof(host), serv, sizeof(serv), 0);
                printf("Command received from %s, host %s\n",inet_ntoa(senderaddress.sin_addr),host);
	}
	

        close(socketb);
	return 0;
}
