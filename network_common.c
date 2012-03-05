#ifndef _network_common_c
#define _network_common_c

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <dirent.h>
#include "global.h"

int hostname_to_ip(char *hostname, char *ip)
{
        struct hostent *he;
        struct in_addr **addr_list;
        int i;
        if ( (he = gethostbyname( hostname ) ) == NULL)
        {
                // get the host info
                printf("Hostname not present\n");
                return -1;
        }

        addr_list = (struct in_addr **) he->h_addr_list;

        for(i = 0; addr_list[i] != NULL; i++)
        {
                //Return the first one;
                strcpy(ip , inet_ntoa(*addr_list[i]) );
                return 0;
        }
        return 1;
}

// 0 if file exists, 1 if file does not exist
static int fexist(char *filename)
{
  struct stat buffer;
  if ( stat(filename, &buffer) ) return 1;
  return 0;
}

int isDir(const char* target)
{
   struct stat statbuf;
   stat(target, &statbuf);
   return S_ISDIR(statbuf.st_mode);
}

//Checks if a datanode is alive..
// 1 - alive.. 0 - dead
int check_nodealive(char *name)
{
	printf("Check nodealive %s\n",name);
	printf("Ping success %d\n",pingsuccess);
	if(name == NULL)
		return 0;
        sendping(name);
        int counter = 0;
        while(pingsuccess != 1)
        {
                usleep(1);
                counter++;
                if(counter > 100)
		{
			printf("poruthathu pothum pongi elu\n");
                        return 0;
		}
        }

        return 1;
}

// Structure to send list of directories.. :)
// I am just creating so that later if I have to add anything, I can add.
struct dirent_detail
{
	char d_name[100];
	int d_type;
	long int d_ino;
};
struct readdir_list
{
	int n;
	//struct dirent_detail dlist[1024];
	//struct dirent de;
};
#endif
