#ifndef _global_h
#define _global_h

#define PING_PORT 8011
#define COMMAND_PORT 8012
#define RESPONSE_PORT 8013
#define ACCESS "access"
#define GETATTR "getattr"
#define DATANODE "datanode"
#define READDIR "readdir"
#define OPEN "open"
#define DNULL 0 
#define EOS '\0'
#define MAXF 200
#define REP  100
#define MAX_PATH 100
char *COMMAND_NAME;
struct stat *st_getattr;
//struct readdir_list *rlist_all[1024];
struct readdir_list *rlist_all;
char readdir_buf[10000];
int access_return = 0;
int open_return = 0;

#endif