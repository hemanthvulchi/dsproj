

#ifndef _network_common_c
#define _network_common_c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

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

#endif
