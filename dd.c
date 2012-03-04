#ifndef _dd_functions
#define _dd_functions
#include<stdio.h> 
#include<malloc.h> 
#include<string.h>
#include<stdlib.h>
#include "global.h"
#include "dd.h"
#include "list.c"
node* prev = DNULL; 
node* fold = DNULL;

void printdata(node* temp)
{
		int i;
		datadef tdata;
		tdata=temp->data;
  	printf("\nNode Number \t:%d \nParent Node \t:%d \n File:%d \nFile Name \t:%s\nData Node \t:%s\nPath \t:%s\n",tdata.nno,tdata.pno,tdata.dflag,tdata.fname,tdata.dname[0],tdata.loc);
	for(i=0;i<REP;i++)
	{
		printf("Dname:%s Dsync:%d   \n",tdata.dname[i],tdata.dsync[i]);
	}
}

/**************************************
Function to get just the file or folder name
**************************************/
void parsepath(char* ans,char* vpath)
{
	int ilen;
	char* stemp;
	stemp=strrchr(vpath,'/');
	ilen=strlen(stemp);
	printf("\n stemp:%s",stemp);
	printf("\n len:%d  ",ilen);
    cstrcpy(ans,stemp,1, ilen-1);
	printf("\n len:%d stemp:%s",ilen,ans);
}

/**************************************
Function to copy data and on '0' length copies the whole source string
**************************************/
void cstrcpy(char *sret,char* stemp,int start, int ilen)
{
	int i=0;	
	if(ilen!=0)
	{
		while(i<=ilen)
		{
			sret[i]=stemp[start+i];
			i++;

		}
		sret[i]=EOS;
	}
	else
	{
		while(stemp[i]!=EOS)
		{
			sret[i]=stemp[start+i];
			i++;
		}
		sret[i]=EOS;		
	}
		
}

/**************************************
Function to find a first occurence character
**************************************/
int findfch(char *stemp, char ctemp)
{
	int i=0;	
	while(stemp[i]!=EOS)
	{
		if(stemp[i]==ctemp)
			return ++i;
		i++;
	}
	return -1;
}

/**************************************
Function to get the root folder name
**************************************/
void parserpath(char* ans,char* vpath)
{
	int ipos;
	char* stemp;
	stemp=vpath;
	ipos=findfch(stemp,'/');
	cstrcpy(ans,stemp,0,ipos-2);
	printf("\n from parserpath stemp:%s ans:%s ipos:%d",stemp,ans,ipos);
}

/**************************************
Function to remove the root folder and give remaining path
**************************************/
void removepath(char* ans,char* vpath)
{
	int ipos;
	char* stemp;
	stemp=vpath;
	ipos=findfch(stemp,'/');
	cstrcpy(ans,stemp,ipos,0);
	printf("\n from removepathstemp:%s ans:%s",stemp,ans);
}
node *create_node(datadef vdata) 
{ 
	int i=0;
	node *new1; 
	new1 = malloc(sizeof(node)); 
	new1->data.dflag=0;
	new1->data.nno=0;
	new1->data.pno=0;
	memset(new1->data.fname,'\0',MAX_PATH);
	for(i=0;i<REP;i++)
		strcpy(new1->data.dname[i],"none");
	memset(new1->data.loc,'\0',MAX_PATH);
	new1->data.dflag=0;
	for(i=0;i<REP;i++)
		new1->data.dsync[i]=0;

	new1->data=vdata;
	new1->next = DNULL; 
	new1->prev = DNULL; 
	new1->fold = DNULL;
	return(new1); 
} 
/************************************
Function not used at present
***********************************/
node *create_dnode(datadef vdata) 
{ 
node *new1; 
new1 = malloc(sizeof(node)); 
new1->data=vdata;
new1->data.dflag=1;
new1->next = DNULL; 
new1->prev = DNULL; 
new1->fold = DNULL;
return(new1); 
} 


void  ninitialize() 
{
int i; 
node *new1;
new1 = malloc(sizeof(node)); 
new1->data.dflag=1;
new1->data.nno=1;
new1->data.pno=0;
strcpy(new1->data.loc,"Root");
for(i=0;i<REP;i++)
	strcpy(new1->data.dname[i],"none");
strcpy(new1->data.dname[0],"Root");
strcpy(new1->data.fname,"home");
new1->next = DNULL; 
new1->prev = DNULL; 
new1->fold = DNULL;
head = new1;
nodeno=1; 
} 
 

/***********************************************
Function Search
***********************************************/
node  *search(char* vpath) 
{ 
	int b=1; 
	node *temp; 
	temp = head;
	printf("\nIn Search vpath:%s",vpath); 
	b=strcmp(vpath,"home");
	if(b==0)
	{
		printf("\nReturning temp %d",b);		
		return temp;
	}
	else
	{
		printf("\nGoing to searchloc %d",b);			
		return searchloc(temp,vpath);
	}
}

/**********************************************
Recursive function used by search
**********************************************/ 
node *searchloc(node* temp,char* vpath)
{
		char tpath[MAX_PATH];
		char foldname[MAX_PATH];
		int iSafe=0;
		printf("\n in searchloc path:%s",vpath);
		//Check if you have reached the folder
		if(findfch(vpath,'/')!=-1)
		{
//			temp=temp->fold;
			//this would give the root folder
			iSafe=0;
			parserpath(foldname,vpath);			
			while(temp!=DNULL)
			{
				if(strcmp(temp->data.fname,foldname)==0) 
				{ 
					temp=temp->fold;
					removepath(tpath,vpath);					
					return searchloc(temp,tpath);
				} 
				else 
				{ 
					temp=temp->next;
				}
				iSafe++;
				if (iSafe > 1024)
				{
					printf("node not found");
				}
			}	
		}
		else
		{
			if(strcmp(vpath,"home")==0)
			{
				return head;
			}
			else if(strcmp((temp->data.fname),"home")==0)
			{
				temp=head->fold;
			}
			while(temp!=DNULL)
			{
				if(strcmp((temp->data.fname),vpath)==0)
					return temp;
  			temp = temp->next; 	
			} 
		}		
	printf("\nError in search, no Node found\n");
	return DNULL;
}

int findlch(char *stemp, char ctemp)
{
	int i=strlen(stemp);	
	while(i>=0)
	{
		if(stemp[i]==ctemp)
			return i;
		i--;
	}
	return -1;
}
/**************************************
This function gives the directory ommiting the filename and the slash
***************************************/
void getdir(char* ans,char* vpath)
{
	int ipos;
	ipos=findlch(vpath,'/');
	if(ipos!=-1)
	{
		cstrcpy(ans,vpath,0,ipos-1);
	}
	else
	{
		strcpy(ans,"home");
	}
}

node *searchlast(char* tpath)
{
  node* tnode;
  printf("\nIn Search Last");
	tnode=search(tpath);
//  printf("\nIn Search Last: After search");	
  printdata(tnode);
//  printf("\n tnode->fold: %d",tnode->fold);
	if(tnode->fold==DNULL)
	{
//		printf("\nSearchlast return tnode");
		return tnode;
	}
	else
	{
//		printf("\nSearchlast else part");	
		tnode=tnode->fold;
		printdata(tnode);
		while(tnode->next!=DNULL)
		{
			tnode=tnode->next;
		}
		return tnode;	
	}	
}

node *addnode(char* vpath,datadef vdata)
{
  node* tnode;
  node* newnode;
	newnode=create_node(vdata);
//	getdir(tpath,vpath);
//	printf("\nAddnode tpath:%s vpath: %s",tpath,vpath);
	tnode=searchlast(vpath);
	printf("\nAddnode: after search last");
	printdata(tnode);
	if (tnode->data.dflag==1)
	{
		tnode->fold=newnode;
		newnode->prev=tnode;
	}
	else
	{
		tnode->next=newnode;
		newnode->prev=tnode;
	}
		newnode->data.pno=tnode->data.nno;	
		newnode->data.nno=++nodeno;
	return newnode;
}

int removefile(char* vpath)
{
	node* tnode;
	node* prev;
	node* next;
	tnode=search(vpath);
	prev=tnode->prev;
	next=tnode->next;
	if (prev->data.dflag==1)
	{
		prev->fold=tnode->next;
	}
	else
	{
		prev->next=tnode->next;
	}

	next->prev=prev;	
	free(tnode);
	return 0;
}


int removefold(char* vpath)
{
	node* tnode;
	node* prev;
	node* next;
	node* temp;
	node* tempn;
	tnode=search(vpath);
	prev=tnode->prev;
	next=tnode->next;
	if (prev->data.dflag==1)
	{
		prev->fold=tnode->next;
	}
	else
	{
		prev->next=tnode->next;
	}
	next->prev=prev;	
	temp=tnode->fold;
	while(temp->next!=DNULL)
	{
		if(temp->data.dflag==1)
		{
			tempn=temp;
			temp=temp->next;	
			removefold(tempn->data.loc);
		}
		else
		{
			tempn=temp;
			temp=temp->next;	
			removefile(tempn->data.loc);
		}
	}
	if(temp->data.dflag==1)
	{
		tempn=temp;
		removefold(tempn->data.loc);
	}
	else
	{
		tempn=temp;
		removefile(tempn->data.loc);
	}

	return 0;
}


node *addfile(char *fname,char *dname,char *loc)
{
	int i;	
	datadef tdata;
	node *tnode;
	tdata.dflag=0;
	tdata.nno=0;
	tdata.pno=0;
	memset(tdata.fname,'\0',MAX_PATH);
	for(i=0;i<REP;i++)
		strcpy(tdata.dname[i],"none");
	memset(tdata.loc,'\0',MAX_PATH);
	tdata.dflag=0;
	for(i=0;i<REP;i++)
		tdata.dsync[i]=0;

	strcpy(tdata.fname,fname);
	strcpy(tdata.dname[0],dname);
	tdata.dsync[0]=1;
	strcpy(tdata.loc,loc);
	tdata.dflag=0;
	printf("\nAddfile loc:%s",loc);
	tnode =	addnode(loc,tdata);
	return tnode;
}

node *addfolder(char *fname,char *dname,char *loc)
{
	int i;
	datadef tdata;
	node *tnode;
	tdata.dflag=0;
	tdata.nno=0;
	tdata.pno=0;
	memset(tdata.fname,'\0',MAX_PATH);
	for(i=0;i<REP;i++)
		strcpy(tdata.dname[i],"none");
	memset(tdata.loc,'\0',MAX_PATH);
	tdata.dflag=0;
	for(i=0;i<REP;i++)
		tdata.dsync[i]=0;
	strcpy(tdata.fname,fname);
	strcpy(tdata.dname[0],dname);
	tdata.dsync[0]=1;
	strcpy(tdata.loc,loc);
	tdata.dflag=1;
	tnode =	addnode(loc,tdata);
	return tnode;
}


char *getdatanode(char* path,char* host)
{
	int i=0,fh=-1;
	node *tnode;
	tnode=search(path);
	if(tnode!=DNULL)
	{
		while(i<REP)
		{
			if((strcmp( tnode->data.dname[i],host)==0) && tnode->data.dsync[i]==1);		
			{
				fh=i;
				break;
			}
			i++;
		}
		if(fh!=-1)
		return tnode->data.dname[fh];
		i=0;
		while(i<REP)
		{
			if( tnode->data.dsync[i]==1);	
			{
				fh=i;				
				break;
			}
			i++;
		}
		if(fh!=-1)
			return tnode->data.dname[fh];


		i=0;
		while(i<REP)
		{
			if(( tnode->data.dsync[i]==2) && (strcmp( tnode->data.dname[i],host)==0) );	
			{
				fh=i;				
				break;
			}
			i++;
		}
		if(fh!=-1)
			return tnode->data.dname[fh];
		i=0;
		while(i<REP)
		{
			if(( tnode->data.dsync[i]==2)  );	
			{
				fh=i;				
				break;
			}
			i++;
		}
		if(fh!=-1)
			return tnode->data.dname[fh];
		
		printf("Error: No data node found \n");
		return "lisp.rutgers.edu";

	}
		//Check if client is a datanode itself

		datanode *dn=datanode_search(host);
		if (dn!=NULL)
		return dn->node;
	
		dn=fetch_datanode();
		if (dn!=NULL)
		return dn->node;		

		printf("No data node found \n");		
		return "lisp.rutgers.edu";
}


////////////////////
int storelist(node* thead,char* fpath)
{
	FILE *ifp, *ofp;
	char *mode = "r";
	char outputFilename[] = "data1.txt";
    int iFlno, iFrn;
    char sFfname[100], sFdn[100];
    	char buffer[1000];
	ifp = fopen("data.txt", "a+");
	printf("Store list\n");
	if (ifp == NULL) 
	{
		fprintf(stderr, "Can't open input file in.list!\n");
		return -1;
	}
	printf("calling recursive store list \n");
	recursestore(ifp,thead);
/*	
	while (fscanf(ifp, "%d %s %s %d", &iFlno,sFfname, sFdn,&iFrn) != EOF) 
	{
		   fprintf(ofp, "%d %s %s %d\n", iFlno,sFfname, sFdn,iFrn);
	}	
*/	

    fclose(ifp);
	return 0;
}

void recursestore(FILE* tfp, node* tnode)
{
	if(tnode==DNULL)
		printf("tnode is null\n");
 	char buffer[1000];
 	char tbuff[1000];
	int i=0;		
    
	printdata(tnode);


	memset(buffer,'\0',1000);
	memset(tbuff,'\0',1000);
	snprintf(tbuff,10,"%d",tnode->data.nno);
	strcat(buffer,tbuff);


	strcat(buffer,",");
	memset(tbuff,'\0',1000);
	snprintf(tbuff,10,"%d",tnode->data.pno);
	strcat(buffer,tbuff);

	strcat(buffer,",");
	strcat(buffer,tnode->data.fname);
	strcat(buffer,",");

	for(i=0;i<REP;i++)
	{
		strcat(buffer,tnode->data.dname[i]);
		strcat(buffer,",");
	}
	strcat(buffer,tnode->data.loc);
	strcat(buffer,",");	
	memset(tbuff,'\0',1000);
	snprintf(tbuff,10,"%d",tnode->data.dflag);
	strcat(buffer,tbuff);
	strcat(buffer,",");
	for(i=0;i<REP;i++)
	{
		memset(tbuff,'\0',100);
		snprintf(tbuff,10,"%d",tnode->data.dsync[i]);
		strcat(buffer,tbuff);
		strcat(buffer,",");	
	}
		strcat(buffer,"ENDD\n");	
		fprintf(tfp, "%s", buffer);
		if(tnode->fold!=DNULL)
		{
	//		tnode=tnode->fold;
			printdata(tnode);
			
			recursestore(tfp,tnode->fold);
		}
		if(tnode->next!=DNULL)
		{

//			tnode=tnode->next;
			printdata(tnode);
			recursestore(tfp,tnode->next);			
		}
	
}

node* recoverlist(char* fpath)
{
FILE* ifp;
char buffer[1000];
char *ptr;
int i=0;
datadef tdata;
node *tnode;
printf("Starting to rebuild file system\n");
ifp = fopen(fpath, "r");
printf("Opening File\n");
if (ifp == NULL) 
	{
		fprintf(stderr, "File not present.\n Intialising database\n");
		ninitialize();
		return head;
	}
	printf("File Opened\n");
	while (fscanf(ifp, "%s", &buffer) != EOF)
	{
		printf("Buffer:%s\n",buffer);
		if(i==0)
		{
			ptr = strtok(buffer,",");			
			tdata.nno=atoi(ptr);
			printf("nno: %d\n",tdata.nno);			
			ptr = strtok(NULL,",");
			tdata.pno=atoi(ptr);
			printf("pno: %d\n",tdata.pno);			
			ptr = strtok(NULL,",");
			strcpy(tdata.fname,ptr);
			printf("fname: %s\n",tdata.fname);
			for(i=0;i<REP;i++)
			{
				ptr = strtok(NULL,",");
				strcpy(tdata.dname[i],(ptr));
				printf("dname: %s i:%d\n",ptr,i);			
			}
			printf("Fourth \n");
			ptr = strtok(NULL,",");
			strcpy(tdata.loc,ptr);
			printf("loc=%s\n",tdata.loc);
			ptr = strtok(NULL,",");
			tdata.dflag=atoi(ptr);
			printf("dflag=%d\n",tdata.dflag);
			for(i=0;i<REP;i++)
			{
				ptr = strtok(NULL,",");
				tdata.dsync[i]=atoi(ptr);
				printf("In Look %d val=%d \n",i,tdata.dsync[i]);
			}
			printf("Opening File- Creating Head\n");
			head=create_node(tdata);
			printdata(head);
			i++;
		}
		else
		{
			printf("Opening File- reading other nodes\n");
			char *ptr = strtok(buffer,",");			
			tdata.nno=atoi(ptr);
			ptr = strtok(NULL,",");
			tdata.pno=atoi(ptr);
			ptr = strtok(NULL,",");
			strcpy(tdata.fname,ptr);
			for(i=0;i<REP;i++)
			{
				ptr = strtok(NULL,",");
				strcpy(tdata.dname[i],(ptr));
			}
			ptr = strtok(NULL,",");
			strcpy(tdata.loc,ptr);
			ptr = strtok(NULL,",");
			tdata.dflag=atoi(ptr);
			for(i=0;i<REP;i++)
			{
				ptr = strtok(NULL,",");
				tdata.dsync[i]=atoi(ptr);
			}
			printf("Opening File- looking for other nodes\n");
			tnode=addnode(tdata.loc,tdata);
			printdata(tnode);
		}
	}
	return head;
	fclose(ifp);
}

int dummymain() 
{ 
printf("inside dummymain");
node *nnode,*tnode,*fnode;
printf("Started\n");
head=recoverlist("data.txt");
/*
ninitialize();
printf("\nInitialized");
nnode=addfile("file1","dpath","home");
printf("added first node");
fnode=addfile("file2","dpath","home");
printdata(head);
printdata(nnode);
printdata(fnode);
printf("\nSearching");
tnode=search("home/file1");
printdata(tnode);;
fnode=addfolder("fold1","dpath2","home");
printdata(fnode);
fnode=addfile("file3","dpath3","home/fold1");
printdata(fnode);
fnode=search("home/fold1/file3");
printdata(fnode);
storelist(head,"dummy");
free(nnode);
free(head);
//*/
printf("My search \n");
fnode=search("home/fold1/file3");
printdata(fnode);

printf("\n");
return 0;
}

#endif
