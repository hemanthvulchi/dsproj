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
	int d_type[50];
	int d_ino[50];
	char replica[50][100];
	int rep[50]; // 0 - not present, 1 - present, 2 or above - not synced.
};
typedef struct _file file_d;

struct _filelist
{
	//file_d FILED;
	char name[100];
        int d_type[50];
        int d_ino[50];
        char replica[50][100];
        int rep[50]; // 0 - not present, 1 - present, 2 or above - not synced.

	struct _filelist *next;	
}*sfile,*firstsfile;
typedef struct _filelist filelist;

void initfilelist(void)
{
        sfile = NULL;
        firstsfile = NULL;
}
filelist * filelist_search(char []);

datanode* datanode_search(char []);
int filenode_insert(char *name, int type, int ino, char *dn)
{
        filelist *p;
        if( (p=filelist_search(name)) == NULL)
        {
                printf("Insert File %s\n",name);
                p = (struct _filelist*) malloc(sizeof(struct _filelist));
		memset(p->name,'\0',100);
                strcpy(p->name,name);
		printf("Name : %s\n",p->name);
		initFile(p);
		int i = getRepcreatenum(p);
		p->rep[i]=1;
                strcpy(p->replica[i],dn);
                p->d_ino[i] = ino;
                p->d_type[i] = type;

                sfile = firstsfile;
                if( sfile == NULL )
                {
                        sfile = p;
                        firstsfile = sfile;
                }
                else
                {
                        while(sfile->next != NULL)
                                sfile = sfile->next;
                        sfile->next = p;
                }
        }
        else
        {
		int i = getRepcreatenum(p);
                p->rep[i] = 1;
                strcpy(p->replica[i],dn);
                p->d_ino[i] = ino;
                p->d_type[i] = type;
        }
	filelist_display();
        //datanode_filewrite();
        return 0;
}
datanode* fetch_datanode();
char* getfiledn(char *name, char *host)
{
        filelist *p;
        if( (p=filelist_search(name)) == NULL)
	{
		datanode *dn = fetch_datanode();
		if(dn == NULL)
			return NULL;
		return dn->node;
	}
	else
	{
		int i = getRepfetchnum(p);
		if( i != -1)
		{
			return p->replica[i];
		}
		else
		{
			i = getRepfetchnum1(p);
			if( i != -1)
			return p->replica[i];
			else
				return NULL;
		}
	}
}

int initFile(filelist *f)
{
	printf("InitFile\n");
	int i = 0;
	for ( i = 0; i < 50; i++)
	{
		f->rep[i] = 0;
	}
}

int getRepcreatenum(filelist *f)
{
	printf("Get rep create num ()\n");
	int i = 0;
	for (i = 0; i < 50; i++)
	{	
		if(f->rep[i] == 0)
			return i;	
	}
	printf("Out of get rep create num\n");
	return -1;
}

int getRepfetchnum(filelist *f)
{
        int i = 0;
        for (i = 0; i < 50; i++)
        {
                if(f->rep[i] == 1)
		{
			datanode *dn = datanode_search(f->replica[i]);
			if (check_nodealive(dn->node) == 1)
                        	return i;
		}
        }
        return -1;
}

int getRepfetchnum1(filelist *f)
{
        int i = 0;
        for (i = 0; i < 50; i++)
        {
                if(f->rep[i] > 1)
		{
			datanode *dn = datanode_search(f->replica[i]);
			if (check_nodealive(dn->node) == 1)
                        	return i;
		}
        }
        return -1;
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
	filenode_insert(DATANODE_DIR,0,0,name);
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

filelist* filelist_search(char name[])
{
	char *aname = malloc(strlen(name) + 1);
	strcpy(aname,name);
	if( aname[strlen(name)-1] == '/')
		aname[strlen(name)-1]='\0';
        filelist *entry = NULL;
        sfile = firstsfile;
        if( sfile == NULL )
        {
                return entry;
        }
        else
        {
                while(sfile != NULL)
                {
                        if( strcmp(sfile->name,name) == 0  || strcmp(sfile->name,aname) == 0)
                        {
				free(aname);
                                return sfile; 
                        }
                        sfile = sfile->next;
                }
        }
	free(aname);
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
				{
				datanode_filewrite();
                		return dlist;
				}
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

//Dont use this..
int remove_fileentry(filelist *sfile1)
{
        printf("REmove fileentry : %s\n", sfile1->name);
        if(sfile1->next == NULL)
        {
                free(sfile1);
        }
        else
        {
                filelist *tmp = sfile1->next;
		strcpy(sfile1->name,sfile1->next->name);
                //strcpy(dlist1->node,dlist1->next->node);
                //strcpy(dlist1->ip,dlist1->next->ip);
                //dlist1->alive = dlist->next->alive;
                //dlist1->next = tmp->next;
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

void filelist_display()
{
        printf("Display Filelist\n");
       	sfile = firstsfile;
        if(sfile == NULL )
        {
                printf("Empty Datanode list\n");
        }
        else
        {
                while(sfile != NULL)
                {
                        printf("name: %s, d_ino: %d, d_type : %d\n", sfile->name, sfile->d_ino, sfile->d_type);
                        sfile = sfile->next;
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

void datanode_sendlist(char *dn_buf)
{
        dlist = firstdlist;
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
                        strcat(dn_buf,",");
                        dlist = dlist->next;
                }
        }
}

#endif
