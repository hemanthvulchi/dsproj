#ifndef _dd_functions
#define _dd_functions
#include<stdio.h> 
#include<malloc.h> 
#include<string.h>
#include<stdlib.h>
#include "dd.h"
node* prev = DNULL; 
node* fold = DNULL;

void printdata(node* temp)
{
		datadef tdata;
		tdata=temp->data;
  	printf("\nNode Number \t:%d \nParent Node \t:%d \n File:%d \nFile Name \t:%s\nData Node \t:%s\nPath \t:%s",tdata.nno,tdata.pno,tdata.dflag,tdata.fname,tdata.dname[0],tdata.loc);
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
node *new1; 
new1 = malloc(sizeof(node)); 
new1->data.dflag=0;
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
node *new1;
new1 = malloc(sizeof(node)); 
new1->data.dflag=1;
new1->data.nno=1;
new1->data.pno=0;
strcpy(new1->data.loc,"Root");
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
	exit(0);
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
		removefile(tempn)->data.loc;
	}

	return 0;
}


node *addfile(char *fname,char *dname,char *loc)
{
	datadef tdata;
	node *tnode;
	strcpy(tdata.fname,fname);
	strcpy(tdata.dname[0],dname);
	strcpy(tdata.loc,loc);
	tdata.dflag=0;
	printf("\nAddfile loc:%s",loc);
	tnode =	addnode(loc,tdata);
	return tnode;
}

node *addfolder(char *fname,char *dname,char *loc)
{
	datadef tdata;
	node *tnode;
	strcpy(tdata.fname,fname);
	strcpy(tdata.dname[0],dname);
	strcpy(tdata.loc,loc);
	tdata.dflag=1;
	tnode =	addnode(loc,tdata);
	return tnode;
}

int dummymain() 
{ 
node *nnode,*tnode,*fnode;
printf("\n Started");
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
free(nnode);
free(head);
printf("\n");
return 0;
}

#endif
