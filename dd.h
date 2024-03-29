#include "global.h"
#ifndef _dd_h
#define _dd_h
typedef struct stdata 
{ 
int nno;       
int pno;
char fname[MAX_PATH];
char dname[REP][SERV_PATH];
char loc[MAX_PATH];
int dflag;
int dsync[REP]; // 1 - Sync  0-Not Set 2-Unsync
} datadef; 
int nodeno;

typedef struct list 
{ 
datadef data; 
struct list *next; 
struct list *prev; 
struct list *fold;
}node; 
node *head = DNULL;  // Head node Root
void printdata(node* temp);
void parsepath(char* ans,char* vpath);
void parserpath(char* ans,char* vpath);
void cstrcpy(char *sret,char* stemp,int start, int ilen);
int findfch(char *stemp, char ctemp);
void removepath(char* ans,char* vpath);
node *create_node(datadef vdata);
node *create_dnode(datadef vdata);
void insert_beg(datadef vdata);
node *search(char* vpath);
node *searchloc(node* temp,char* vpath);
int findlch(char *stemp, char ctemp);
void getdir(char* ans,char* vpath);
node *addnode(char* vpath,datadef vdata);
void  ninitialize(); 
node *addfile(char *fname,char *dname,char *loc);
node *addfolder(char *fname,char *dname,char *loc);
int removefile(char* vpath);
int removefold(char* vpath);
char *getdatanode(char* path,char* host);
int dummymain();
int storelist(node* thead,char* fpath);
void recursestore(FILE* tfp, node* tnode);
node* recoverlist(char* fpath);
#endif
