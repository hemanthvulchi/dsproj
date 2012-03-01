#ifndef __LIST_H
#define __LIST_H

//typedef struct node file_info_list;
typedef struct node server_list;
typedef struct node list_tail;
typedef struct node list_head;
struct node 
{
  char  *server_addr;
  unsigned int alength;
  void * data;
  server_list * next;
  server_list * prev;
};

/// methods to cache size information. right now we use simple link list.
int insert_info_ptr(list_head *, list_tail **, const char* addr,
                    unsigned int addr_length, void * data);
int delete_info(list_head *, list_tail **, const char * addr,
                unsigned int addr_info);
int remove_info(list_head *,  list_tail **, const char * addr,
                unsigned int addr_length);
int remove_info_ptr(list_head *,  list_tail **, void *);
int change_info_ptr(list_head *, list_tail **, const char * addr,
                    unsigned int addr_legnth, void * data);
int get_info_ptr(list_head *,  list_tail **, const char * addr,
                 unsigned int addr_length, void **pointer_to_data);

void free_list(list_head *);
int init_list_head(list_head *);

#endif
