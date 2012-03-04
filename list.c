/*
*	I will basically write all my lists in here..
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#ifndef _datanode_functions
#define _datanode_functions
#include "network_common.c"
struct _datanode
{
	char *node;
	char *ip;
	int alive; // 1 -alive, 0 - dead
	struct _datanode *next;
}*dlist,*firstdlist;
typedef struct _datanode datanode;

//There should be one for file also
struct _file
{
	char name[100];
};
typedef struct _file file_d;

struct _filelist
{
	file_d FILED;
	struct _filelist *next;	
}*sfile,*firstsfile;

void initfilelist(void)
{
        sfile = NULL;
        firstsfile = NULL;
}

int writednconfig(char *buf)
{
	FILE *fp = fopen(namenode_configfile, "w");
	if (fp == NULL)
	{
		printf("error writednconfig()\n");
	}
	fwrite(buf,strlen(buf),1,fp);
	fclose(fp);
	return 0;
}

int readdnconfig()
{
	FILE *fp = fopen(namenode_configfile, "r");
	if (fp == NULL)
	{
		printf("error readnconfig()\n");
	}
	char line[4096];
	while (fgets(line,sizeof(line),fp) != NULL)
	{
		printf("Line : %s\n",line);
		char *ptr = strtok(line,",");
		char node[20];
		char ip[20];
		strcpy(node,ptr);
		ptr = strtok(NULL,",");
		strcpy(ip,ptr);
		ptr = strtok(NULL,",\n");
		int alive = atoi(ptr);
		datanode_insert(node,ip,alive);
		/*while(ptr != '\n' || ptr != '\0')
		{
			
		}*/
	}
	fclose(fp);
	return 0;
}

void initdatanodelist(void)
{
	printf("In initdatanodelist\n");
	dlist = NULL;
	firstdlist = NULL;
	char *p = getenv("USER");
	if (p == NULL)
	{
		printf("Error detecting user\n");
		exit(0);
	}
	char buf[100];
	memset(buf,'\0',100);
	strcpy(buf,"/tmp/.cs545/");
	printf("Buf : %s\n",buf);
	int res;
	res = mkdir(buf,0700);
        if (res == -1)
	{
               if(errno != EEXIST)
		{
                	printf("Error dnlist init 1\n"); 
			exit(1);           
		}
	}

	strcat(buf,p);
	printf("buf %s\n",buf);
	res = mkdir(buf,0700);
	if (res == -1)
	{
		if(errno != EEXIST)
		{
		printf("Error dnlist init 2\n");
		exit(1);	
		}
	}
	memset(namenode_configfile,'\0',120);
	strcpy(namenode_configfile,"");
	strcat(namenode_configfile,buf);
	memset(buf,'\0',100);
	strcat(namenode_configfile,"/dnconfig");
	printf("Namenode configfile, %s\n",namenode_configfile);
	if(fexist(namenode_configfile) == 1)
	{
		printf("buf : %s\n");
		writednconfig("");
	}
	else
	{
		printf("read from a file and then load it..\n");
		readdnconfig();
	}
}

datanode* datanode_search(char []);

int datanode_insert(char name[], char ip[],int alive)
{
        datanode *p;
        if( (p=datanode_search(name)) == NULL)
        {
		printf("Insert Datanode\n");
                p = (struct _datanode*) malloc(sizeof(struct _datanode));
		p->node = (char *) malloc(strlen(name)+1);
                strcpy(p->node, name);
		p->ip = (char *) malloc(strlen(ip)+1);
		strcpy(p->ip, ip);
		p->alive = alive;
                p->next = NULL;
                dlist = firstdlist;
                if( dlist == NULL )
                {
                        dlist = p;
                        firstdlist = dlist;
                }
                else
                {
                        while(dlist->next != NULL)
                                dlist = dlist->next;
                        dlist->next = p;
                }
        }
	else
	{
		p->alive = 1;
	}
	datanode_filewrite();
        return 0;
}

datanode* datanode_search(char name[])
{
	printf("I am in datanode search\n");
        datanode *entry = NULL;
        dlist = firstdlist;
        if( dlist == NULL )
        {
                return entry;
        }
        else
        {
                while(dlist != NULL)
                {
                        if( strcmp(dlist->node,name) == 0 )
                        {
                                return dlist;
                        }
                        dlist = dlist->next;
                }
        }
	printf("End of it\n");
        return entry;
}

datanode* fetch_datanode()
{	
	printf("Fetch datanode\n");
	datanode *entry = NULL;
	dlist = firstdlist;
        if( dlist == NULL )
        {
                return entry;
        }
        else
        {
                while(dlist != NULL)
                {
			if(dlist->alive != 0)
			{
				if(check_nodealive(dlist->node) == 1)
                		return dlist;
				else dlist->alive = 0;
			}
			dlist = dlist->next;
                }
        }
	datanode_filewrite();
	printf("\n\nend of fetch - No datanode found\n\n\n");
        return entry;
}

int remove_datanode(datanode *dlist1)
{
	printf("REmove datanode : %s\n", dlist1->node);
	if(dlist1->next == NULL)
	{
		free(dlist1);
	}
	else
	{
		datanode *tmp = dlist1->next;
		strcpy(dlist1->node,dlist1->next->node);
		strcpy(dlist1->ip,dlist1->next->ip);
		dlist1->alive = dlist->next->alive;
		dlist1->next = tmp->next;
		printf("Before free\n");
		free(tmp);
	}
        return 0;
}

void datanode_display()
{
	printf("Display Datanode\n");
        dlist = firstdlist;
        if( dlist == NULL )
        {
                printf("Empty Datanode list\n");
        }
        else
        {
                while(dlist != NULL)
                {
                        printf("name: %s, ip: %s, alive : %d\n", dlist->node, dlist->ip, dlist->alive);
                        dlist = dlist->next;
                }
        }
	printf("End of display\n");
}

void datanode_filewrite()
{
	printf("Filewrite Datanode\n");
        dlist = firstdlist;
	char dn_buf[10000];
	memset(dn_buf,'\0',10000);
	strcpy(dn_buf,"");
	char buffer[30];
	memset(buffer,'\0',30);
        if( dlist == NULL )
        {
                printf("Empty Datanode list\n");
        }
        else
        {
                while(dlist != NULL)
                {
			strcat(dn_buf,dlist->node);
			strcat(dn_buf,",");
			strcat(dn_buf,dlist->ip);
			strcat(dn_buf,",");

			memset(buffer,'\0',30);
			snprintf(buffer,10,"%d",dlist->alive);
			strcat(dn_buf,buffer);
			strcat(dn_buf,"\n");
                        dlist = dlist->next;
                }
        }
	writednconfig(dn_buf);
}

#endif
