/*
*	I will basically write all my lists in here..
*/

#ifndef _datanode_functions
#define _datanode_functions
struct _datanode
{
	char *node;
	char *ip;
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

void initdatanodelist(void)
{
	dlist = NULL;
	firstdlist = NULL;
}

datanode* datanode_search(char []);

int datanode_insert(char name[], char ip[])
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
        return 0;
}

datanode* datanode_search(char name[])
{
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
        return entry;
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
                        printf("name: %s, ip: %s\n", dlist->node, dlist->ip);
                        dlist = dlist->next;
                }
        }
}
#endif
